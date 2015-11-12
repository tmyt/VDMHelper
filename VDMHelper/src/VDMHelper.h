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
		bool isConsoleWindowClass(HWND hwnd);
		bool perfomMoveWindow(HWND hwnd, GUID* pguid);

		std::mutex mVdmLock;
		IVirtualDesktopManager* mpVdm;
		bool mInitializationFailed;

		UINT mMoveWindowToDesktopMessage;
	};

#pragma pack(push, 1)
	struct Shared32
	{
		unsigned int hwnd;
		GUID guid;
	};

	struct Shared64
	{
		HWND hwnd;
		GUID guid;
	};
#pragma pack(pop)
#ifdef _WIN64
	typedef Shared64 SharedValue;
#else
	typedef Shared32 SharedValue;
#endif
}