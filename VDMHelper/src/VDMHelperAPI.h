#pragma once

#include <Windows.h>

// Message Identifier for MoveWindowToDesktop proxy
#define RequestMoveWindowToDesktop _T("VDM.Helper.RequestMoveWindowToDesktop")

LRESULT CALLBACK VDMHookProc1(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK VDMHookProc2(int nCode, WPARAM wParam, LPARAM lParam);
LPVOID CALLBACK VDMAllocGuid(HWND hwnd, const GUID* from);
LRESULT CALLBACK VDMReleaseGuid(HWND hwnd, LPVOID ptr);
LRESULT CALLBACK VDMInject(HWND hwnd, const GUID* from);