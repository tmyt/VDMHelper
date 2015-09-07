#pragma once

#include <Windows.h>

// Message Identifier for MoveWindowToDesktop proxy
#define RequestMoveWindowToDesktop _T("VDM.Helper.RequestMoveWindowToDesktop")

LRESULT CALLBACK VDMHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LPVOID CALLBACK VDMAllocGuid(HWND hwnd, const GUID* from);
LRESULT CALLBACK VDMReleaseGuid(HWND hwnd, LPVOID ptr);