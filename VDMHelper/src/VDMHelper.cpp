// VDMHelper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "VDMHelper.h"

#include <thread>

namespace VDM
{
	bool Helper::init()
	{
		mMoveWindowToDesktopMessage = RegisterWindowMessage(RequestMoveWindowToDesktop);
		return true;
	}

	bool Helper::deinit()
	{
		return true;
	}

	bool Helper::create()
	{
		auto pVdm = getVdm();
		if (!pVdm) {
			CoInitialize(nullptr);
			pVdm = getVdm();
			if (!pVdm) return (mInitializationFailed = true), false;
		}
		mpVdm = pVdm;
		return true;
	}

	IVirtualDesktopManager* Helper::getVdm()
	{
		// initialize
		IVirtualDesktopManager* pVdm;
		auto hr = CoCreateInstance(CLSID_VirtualDesktopManager, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pVdm));
		if (FAILED(hr)) return nullptr;
		return pVdm;
	}

	bool Helper::isConsoleWindowClass(HWND hwnd)
	{
		TCHAR szClassName[32] = { 0 };
		GetClassName(hwnd, szClassName, 31);
		return _tcscmp(szClassName, _T("ConsoleWindowClass")) == 0;
	}

	bool Helper::perfomMoveWindow(HWND hwnd, GUID* pguid)
	{
		std::lock_guard<std::mutex> lock(mVdmLock); // scoped lock
		if (!mpVdm && !create()) return false;
		mpVdm->MoveWindowToDesktop(hwnd, *pguid);
		SetForegroundWindow(hwnd);
		return true;
	}

	bool Helper::process(HWND hwnd, UINT msg, ::WPARAM wParam, ::LPARAM lParam)
	{
		if (msg != mMoveWindowToDesktopMessage) return false;
		if (isConsoleWindowClass(hwnd)) return false; // ignore console window
		GUID* pguid = reinterpret_cast<GUID*>(lParam);
		if (mInitializationFailed) return false;
		switch (wParam) {
		case 0:
			pguid = new GUID();
			memcpy(pguid, reinterpret_cast<void*>(lParam), sizeof(GUID));
#ifdef _WIN64
			// 64 bit process can move in non UI context.
			std::thread([hwnd, pguid, this]() {
				perfomMoveWindow(hwnd, pguid);
				delete pguid;
			}).detach();
			break;
#else
			// 32 bit process must be in UI context and non SendMessage process.
			PostMessage(hwnd, msg, 1, reinterpret_cast<LPARAM>(pguid));
			break;
		case 1:
			perfomMoveWindow(hwnd, pguid);
			delete pguid;
			break;
#endif
		}
		return true;
	}
}
