#include <windows.h>
#include "resource.h"
#include "TrayIcon.h"

static const wchar_t* WINDOW_CLASS = L"TrayAppWindowClass";
static const wchar_t* WINDOW_TITLE = L"TrayApp";

// ---------------- Меню главного окна ----------------
static HMENU CreateMainMenu() {
    HMENU hMenuBar  = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXIT, L"Exit");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    return hMenuBar;
}

// ---------------- Процедура окна ----------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    UINT tbCreated = GetTaskbarCreatedMessage();
    if (tbCreated != 0 && msg == tbCreated) {
        AddTrayIcon(hWnd);
        return 0;
    }

    switch (msg) {
    case WM_CREATE:
        SetMenu(hWnd, CreateMainMenu());
        return 0;

    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP) {
            ShowMainWindow(hWnd);
        } else if (lParam == WM_RBUTTONUP) {
            ShowContextMenu(hWnd);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_OPEN:
            ShowMainWindow(hWnd);
            return 0;
        case ID_TRAY_EXIT:
        case ID_FILE_EXIT:
            DestroyWindow(hWnd);
            return 0;
        }
        return 0;

    case WM_CLOSE:
        // Скрываем окно — приложение продолжает работать в трее
        ShowWindow(hWnd, SW_HIDE);
        return 0;

    case WM_DESTROY:
        RemoveTrayIcon(hWnd);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ---------------- Точка входа ----------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow) {
    // === ЕДИНСТВЕННЫЙ ЭКЗЕМПЛЯР (до всего остального!) ===
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"Global\\TrayApp_SingleInstance_Mutex");
    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    InitTrayMessages();

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = WINDOW_CLASS;
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hIconSm       = wc.hIcon;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc)) {
        CloseHandle(hMutex);
        return 1;
    }

    HWND hWnd = CreateWindowExW(
        0, WINDOW_CLASS, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 400,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        CloseHandle(hMutex);
        return 1;
    }

    AddTrayIcon(hWnd);

    // Флаг /silent — запуск без показа окна
    bool silent = wcsstr(lpCmdLine, L"/silent") != nullptr;

    if (!silent && nCmdShow != SW_HIDE) {
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (hMutex) CloseHandle(hMutex);
    return (int)msg.wParam;
}