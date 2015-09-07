#pragma once

// Refer: http://www.cyberforum.ru/blogs/105416/blog3671.html
#include "VirtualDesktops.h"

#include "VDMHelperAPI.h"

namespace VDM
{
	class Helper
	{
	public:
		Helper() : mMoveWindowToDesktopMessage(0), mpVdm(0) {}

		bool init();
		bool deinit();

		bool process(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		bool tryInit();

		UINT mMoveWindowToDesktopMessage;

		IVirtualDesktopManager* mpVdm;
		bool mInitializationFailed;
	};
}