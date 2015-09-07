#pragma once

#include <tchar.h>

// Refer: http://www.cyberforum.ru/blogs/105416/blog3671.html
#include "VirtualDesktops.h"

// Message Identifier for MoveWindowToDesktop proxy
#define RequestMoveWindowToDesktop _T("VDM.Helper.RequestMoveWindowToDesktop")

namespace VDM
{
	class Helper
	{
	public:
		Helper(): mMoveWindowToDesktopMessage(0), mpVdm(0) {}

		bool init();
		bool deinit();

		bool process(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		UINT mMoveWindowToDesktopMessage;

		IVirtualDesktopManager* mpVdm;
	};
}