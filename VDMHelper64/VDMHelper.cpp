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

	IVirtualDesktopManager* Helper::getVdm()
	{
		// initialize
		CoInitialize(nullptr);
		CComPtr<IServiceProvider> pServiceProvider;
		auto hr = CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pServiceProvider));
		if (FAILED(hr)) return nullptr;
		IVirtualDesktopManager* pVdm;
		hr = pServiceProvider->QueryService(__uuidof(IVirtualDesktopManager), &pVdm);
		if (FAILED(hr)) return nullptr;
		return pVdm;
	}

	bool Helper::process(HWND hwnd, UINT msg, ::WPARAM wParam, ::LPARAM lParam)
	{
		if (msg != mMoveWindowToDesktopMessage) return false;
		GUID* pguid = new GUID();
		memcpy(pguid, reinterpret_cast<void*>(lParam), sizeof(GUID));
		std::thread([&](GUID* g)
		{
			CoInitialize(nullptr);
			auto pVdm = getVdm();
			if(pVdm)
			{
				pVdm->MoveWindowToDesktop(hwnd, *g);
				pVdm->Release();
			}
			CoUninitialize();
			delete g;
		}, pguid).detach();
		return true;
	}
}
