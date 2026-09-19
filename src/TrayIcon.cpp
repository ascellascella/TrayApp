#include "TrayIcon.h"
#include "resource.h"
#include <shellapi.h>

static UINT g_WM_TASKBARCREATED = 0;

void InitTrayMessages() {
    g_WM_TASKBARCREATED = RegisterWindowMessageW(L"TaskbarCreated");
}

UINT GetTaskbarCreatedMessage() {
    return g_WM_TASKBARCREATED;
}

static void FillNid(NOTIFYICONDATAW& nid, HWND hWnd) {
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd   = hWnd;
    nid.uID    = IDI_APP_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon  = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_APP_ICON));
    wcscpy_s(nid.szTip, L"TrayApp");
}

bool AddTrayIcon(HWND hWnd) {
    NOTIFYICONDATAW nid;
    FillNid(nid, hWnd);
    return Shell_NotifyIconW(NIM_ADD, &nid) == TRUE;
}

void RemoveTrayIcon(HWND hWnd) {
    NOTIFYICONDATAW nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd   = hWnd;
    nid.uID    = IDI_APP_ICON;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_OPEN, L"Open");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    // ОБЯЗАТЕЛЬНО, иначе меню может не реагировать на клики
    SetForegroundWindow(hWnd);

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

void ShowMainWindow(HWND hWnd) {
    ShowWindow(hWnd, SW_SHOW);
    ShowWindow(hWnd, SW_RESTORE);
    SetForegroundWindow(hWnd);
}
