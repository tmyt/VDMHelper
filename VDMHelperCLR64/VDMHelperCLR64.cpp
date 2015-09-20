// This is the main DLL file.

#include "stdafx.h"

#include "VDMHelperCLR64.h"

#include "../VDMHelper64/VDMHelperAPI.h"

#define GPA(_Mod, _Name) _Name = (decltype(::_Name)*)::GetProcAddress(_Mod, #_Name)

namespace VDMHelperCLR
{
	bool IsWow64Process(HWND hwnd)
	{
		DWORD pid;
		BOOL isWow64;
		GetWindowThreadProcessId(hwnd, &pid);
		auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
		::IsWow64Process(hProcess, &isWow64);
		CloseHandle(hProcess);
		return isWow64 != 0;
	}

	VdmHelper::VdmHelper() : hCWPHook(0), hGMHook(0)
	{
		hvdm = ::LoadLibrary(_T("VDMHelper64.dll"));
		GPA(hvdm, VDMHookProc1);
		GPA(hvdm, VDMHookProc2);
		GPA(hvdm, VDMAllocGuid);
		GPA(hvdm, VDMReleaseGuid);
		GPA(hvdm, VDMInject);
		RequestMoveWindowToDesktopMessage = RegisterWindowMessage(RequestMoveWindowToDesktop);
	}

	VdmHelper::~VdmHelper()
	{
		DeInit();
		FreeLibrary(hvdm);
	}

	bool VdmHelper::Init()
	{
		if (!hvdm) throw gcnew System::InvalidOperationException("Initialization failed");
		hCWPHook = SetWindowsHookEx(WH_CALLWNDPROCRET, VDMHookProc1, hvdm, 0);
		hGMHook = SetWindowsHookEx(WH_CALLWNDPROCRET, VDMHookProc2, hvdm, 0);
		PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
		// start 32bit helper process
		::ShellExecute(nullptr, nullptr, _T("InjectDll32.exe"), nullptr, nullptr, 0);
		return hCWPHook != 0;
	}

	bool VdmHelper::DeInit()
	{
		if (!hCWPHook)return false;
		UnhookWindowsHookEx(hCWPHook);
		UnhookWindowsHookEx(hGMHook);
		PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
		hCWPHook = 0;
		hGMHook = 0;
		// stop 32bit helper process
		auto hwnd = ::FindWindow(_T("VDM.InjectDLL32.Class"), nullptr);
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		return true;
	}

	bool VdmHelper::MoveWindowToDesktop(IntPtr topLevelWindow, Guid desktopId)
	{
		// convert System.Guid to GUID
		auto bytes = desktopId.ToByteArray();
		pin_ptr<unsigned char> pbytes = &bytes[0];
		GUID dest;
		memcpy(&dest, pbytes, sizeof(GUID));
		// allocate & request
		auto hwnd = (HWND)topLevelWindow.ToPointer();
		LPVOID rGuid = VDMAllocGuid(hwnd, &dest);
		if (isConsoleWindowClass(hwnd)) {
			if (IsWow64Process(hwnd)) return false;
			VDMInject(hwnd, &dest);
		}
		else {
			SendMessage(hwnd, RequestMoveWindowToDesktopMessage, 0, (LPARAM)rGuid);
		}
		VDMReleaseGuid(hwnd, rGuid);
		return true;
	}

	bool VdmHelper::isConsoleWindowClass(HWND hwnd)
	{
		TCHAR szClassName[32] = { 0 };
		GetClassName(hwnd, szClassName, 31);
		return _tcscmp(szClassName, _T("ConsoleWindowClass")) == 0;
	}
}