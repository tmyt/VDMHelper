#pragma once

#define WIN32_MEAN_AND_LEAN

#include <Windows.h>
#include <ShObjIdl.h>

#include <mutex>

#include "VDMHelperAPI.h"

namespace VDM
{
	class Helper
	{
	public:
		Helper() : mMoveWindowToDesktopMessage(0) {}

		bool init();
		bool deinit();

		bool create();
		bool process(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		IVirtualDesktopManager* Helper::getVdm();

		std::mutex mVdmLock;
		IVirtualDesktopManager* mpVdm;
		bool mInitializationFailed;

		UINT mMoveWindowToDesktopMessage;
	};
}