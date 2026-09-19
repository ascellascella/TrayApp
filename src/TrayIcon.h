#pragma once
#include <windows.h>

void InitTrayMessages();
UINT GetTaskbarCreatedMessage();
bool AddTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowContextMenu(HWND hWnd);
void ShowMainWindow(HWND hWnd);
