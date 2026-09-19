#include "session.h"
#include <userenv.h>
#include <wtsapi32.h>
#include <set>

#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "wtsapi32.lib")

static std::set<DWORD> g_launchedPids;

DWORD LaunchInSession(DWORD sessionId, const std::wstring& exePath) {
    HANDLE hToken = NULL;
    if (!WTSQueryUserToken(sessionId, &hToken)) {
        return 0;
    }

    LPVOID lpEnv = NULL;
    CreateEnvironmentBlock(&lpEnv, hToken, FALSE);

    wchar_t cmdLine[1024];
    swprintf_s(cmdLine, L"\"%s\" /parent-pid %lu /silent",
               exePath.c_str(), GetCurrentProcessId());

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.lpDesktop = const_cast<LPWSTR>(L"winsta0\\default");
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessAsUserW(
        hToken, NULL, cmdLine,
        NULL, NULL, FALSE,
        CREATE_UNICODE_ENVIRONMENT | NORMAL_PRIORITY_CLASS,
        lpEnv, NULL, &si, &pi
    );

    if (lpEnv) DestroyEnvironmentBlock(lpEnv);
    CloseHandle(hToken);

    if (!ok) return 0;

    DWORD pid = pi.dwProcessId;
    g_launchedPids.insert(pid);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return pid;
}

void LaunchInAllActiveSessions(const std::wstring& exePath) {
    PWTS_SESSION_INFOW pSessions = NULL;
    DWORD count = 0;
    if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessions, &count))
        return;

    for (DWORD i = 0; i < count; ++i) {
        DWORD sid = pSessions[i].SessionId;
        if (sid == 0) continue;
        if (pSessions[i].State != WTSActive) continue;
        LaunchInSession(sid, exePath);
    }
    WTSFreeMemory(pSessions);
}

void TerminateAllLaunched() {
    for (DWORD pid : g_launchedPids) {
        HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProc) {
            TerminateProcess(hProc, 0);
            CloseHandle(hProc);
        }
    }
    g_launchedPids.clear();
}