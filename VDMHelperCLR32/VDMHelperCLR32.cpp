// This is the main DLL file.

#include "stdafx.h"

#include "VDMHelperCLR32.h"

#include "../VDMHelper32/VDMHelperAPI.h"

#define GPA(_Mod, _Name) _Name = (decltype(::_Name)*)::GetProcAddress(_Mod, #_Name)

namespace VDMHelperCLR
{
	VdmHelper::VdmHelper() : hHook(0)
	{
		hvdm = ::LoadLibrary(_T("VDMHelper32.dll"));
		GPA(hvdm, VDMHookProc);
		GPA(hvdm, VDMAllocGuid);
		GPA(hvdm, VDMReleaseGuid);
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
		hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, VDMHookProc, hvdm, 0);
		PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
		return hHook != 0;
	}

	bool VdmHelper::DeInit()
	{
		if (!hHook)return false;
		UnhookWindowsHookEx(hHook);
		PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
		hHook = 0;
		return true;
	}

	void VdmHelper::MoveWindowToDesktop(IntPtr topLevelWindow, Guid desktopId)
	{
		BOOL isRunningOnX64 = FALSE;
		::IsWow64Process(GetCurrentProcess(), &isRunningOnX64);
		if(isRunningOnX64)
		{
			throw gcnew System::InvalidOperationException("You need call via 64bit dll");
		}
		// convert System.Guid to GUID
		auto bytes = desktopId.ToByteArray();
		pin_ptr<unsigned char> pbytes = &bytes[0];
		GUID dest;
		memcpy(&dest, pbytes, sizeof(GUID));
		// allocate & request
		auto hwnd = (HWND)topLevelWindow.ToPointer();
		LPVOID rGuid = VDMAllocGuid(hwnd, &dest);
		SendMessage(hwnd, RequestMoveWindowToDesktopMessage, 0, (LPARAM)rGuid);
		VDMReleaseGuid(hwnd, rGuid);
	}
}