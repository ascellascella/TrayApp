#include <windows.h>
#include <wtsapi32.h>
#include <string>
#include "session.h"
#include "rpc.h"

#pragma comment(lib, "wtsapi32.lib")

static SERVICE_STATUS_HANDLE g_hStatusHandle = NULL;
static SERVICE_STATUS g_Status = {};
static HANDLE g_hStopEvent = NULL;
static HANDLE g_hRpcThread = NULL;
static std::wstring g_exePath;

// ---------- Прототипы ----------
void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
DWORD WINAPI ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType,
                                   LPVOID lpEventData, LPVOID lpContext);
DWORD WINAPI RpcServerThread(LPVOID lpParam);

// ---------- Путь к GUI (рядом со службой) ----------
static std::wstring GetGuiExePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring s(path);
    size_t pos = s.find_last_of(L"\\");
    if (pos != std::wstring::npos) {
        s = s.substr(0, pos + 1) + L"TrayApp.exe";
    }
    return s;
}

// ---------- Точка входа ----------
int wmain(int argc, wchar_t* argv[]) {
    (void)argc;
    (void)argv;

    SERVICE_TABLE_ENTRYW table[] = {
        { (LPWSTR)L"TrayAppService", (LPSERVICE_MAIN_FUNCTIONW)ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcherW(table)) {
        return 1;
    }
    return 0;
}

// ---------- ServiceMain ----------
void WINAPI ServiceMain(DWORD argc, LPWSTR* argv) {
    (void)argc;
    (void)argv;

    g_hStatusHandle = RegisterServiceCtrlHandlerExW(
        L"TrayAppService", ServiceCtrlHandlerEx, NULL);
    if (!g_hStatusHandle) return;

    g_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_Status.dwCurrentState = SERVICE_START_PENDING;
    g_Status.dwControlsAccepted = 0;
    g_Status.dwWin32ExitCode = NO_ERROR;
    g_Status.dwCheckPoint = 0;
    SetServiceStatus(g_hStatusHandle, &g_Status);

    g_exePath = GetGuiExePath();
    g_hStopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    // RPC-сервер в отдельном потоке
    g_hRpcThread = CreateThread(NULL, 0, RpcServerThread, NULL, 0, NULL);

    // Запускаем GUI во всех активных сессиях
    LaunchInAllActiveSessions(g_exePath);

    // RUNNING. Stop и Shutdown НЕ разрешены (требование 3)
    g_Status.dwCurrentState = SERVICE_RUNNING;
    g_Status.dwControlsAccepted = SERVICE_ACCEPT_SESSIONCHANGE;
    SetServiceStatus(g_hStatusHandle, &g_Status);

    // Ждём сигнала остановки от RPC
    WaitForSingleObject(g_hStopEvent, INFINITE);

    // ---------- Остановка ----------
    g_Status.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(g_hStatusHandle, &g_Status);

    StopRpcServer();
    if (g_hRpcThread) {
        WaitForSingleObject(g_hRpcThread, 5000);
        CloseHandle(g_hRpcThread);
    }

    TerminateAllLaunched();

    CloseHandle(g_hStopEvent);
    g_hStopEvent = NULL;

    g_Status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_hStatusHandle, &g_Status);
}

// ---------- Обработчик событий службы ----------
DWORD WINAPI ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType,
                                   LPVOID lpEventData, LPVOID lpContext) {
    (void)lpContext;

    if (dwControl == SERVICE_CONTROL_SESSIONCHANGE) {
        if (dwEventType == WTS_SESSION_LOGON) {
            WTSSESSION_NOTIFICATION* pNotif = (WTSSESSION_NOTIFICATION*)lpEventData;
            if (pNotif) {
                LaunchInSession(pNotif->dwSessionId, g_exePath);
            }
        }
    }
    return NO_ERROR;
}

// ---------- Поток RPC-сервера ----------
DWORD WINAPI RpcServerThread(LPVOID lpParam) {
    (void)lpParam;
    StartRpcServer(g_hStopEvent);
    return 0;
}