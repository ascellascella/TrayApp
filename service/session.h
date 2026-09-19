#pragma once
#include <windows.h>
#include <string>

DWORD LaunchInSession(DWORD sessionId, const std::wstring& exePath);
void LaunchInAllActiveSessions(const std::wstring& exePath);
void TerminateAllLaunched();