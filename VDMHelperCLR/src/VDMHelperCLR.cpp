// This is the main DLL file.

#include "stdafx.h"

#include "VDMHelperCLR.h"

#include "../../VDMHelper/src/VDMHelperAPI.h"

#define GPA(_Mod, _Name) _Name = (decltype(::_Name)*)::GetProcAddress(_Mod, #_Name)

namespace VDMHelperCLR
{
	VdmHelper::VdmHelper() : hCWPHook(0), hGMHook(0)
	{
#ifdef _WIN64
		hvdm = ::LoadLibrary(_T("VDMHelper64.dll"));
#else
		hvdm = ::LoadLibrary(_T("VDMHelper32.dll"));
#endif
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
		hGMHook = SetWindowsHookEx(WH_GETMESSAGE, VDMHookProc2, hvdm, 0);
		PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
#ifdef _WIN64
		// start 32bit helper process
		::ShellExecute(nullptr, nullptr, _T("InjectDll32.exe"), nullptr, nullptr, 0);
#endif
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
#ifdef _WIN64
		// stop 32bit helper process
		auto hwnd = ::FindWindow(_T("VDM.InjectDLL32.Class"), nullptr);
		SendMessage(hwnd, WM_CLOSE, 0, 0);
#endif
		return true;
	}

	bool VdmHelper::MoveWindowToDesktop(IntPtr topLevelWindow, Guid desktopId)
	{
#ifndef _WIN64
		BOOL isRunningOnX64 = FALSE;
		::IsWow64Process(GetCurrentProcess(), &isRunningOnX64);
		if(isRunningOnX64)
		{
			throw gcnew System::InvalidOperationException("You need call via 64bit dll");
		}
#endif
		// convert System.Guid to GUID
		auto bytes = desktopId.ToByteArray();
		pin_ptr<unsigned char> pbytes = &bytes[0];
		GUID dest;
		memcpy(&dest, pbytes, sizeof(GUID));
		// allocate & request
		auto hwnd = (HWND)topLevelWindow.ToPointer();
		LPVOID rGuid = VDMAllocGuid(hwnd, &dest);
		if (isConsoleWindowClass(hwnd)) {
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