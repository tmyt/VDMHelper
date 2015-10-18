// InjectDll64.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "../../VDMHelper/src/VDMHelperAPI.h"

#ifdef _WIN64
#define HELPER_LIBRARY _T("VDMHelper64.dll")
#define WINDOW_CLASS _T("VDM.InjectDLL64.Class")
#else
#define HELPER_LIBRARY _T("VDMHelper32.dll")
#define WINDOW_CLASS _T("VDM.InjectDLL32.Class")
#endif

// Global Variables:
HINSTANCE hInst;                                // current instance
HHOOK hCWPHook, hGMHook;
HMODULE hVdm;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

bool Init();
bool DeInit();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = WINDOW_CLASS;

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(WINDOW_CLASS, nullptr, 0,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		Init();
		break;
	case WM_DESTROY:
		DeInit();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool Init()
{
	hVdm = LoadLibrary(HELPER_LIBRARY);
	auto proc1 = reinterpret_cast<decltype(::VDMHookProc1)*>(::GetProcAddress(hVdm, "VDMHookProc1"));
	auto proc2 = reinterpret_cast<decltype(::VDMHookProc2)*>(::GetProcAddress(hVdm, "VDMHookProc2"));
	hCWPHook = SetWindowsHookEx(WH_CALLWNDPROCRET, proc1, hVdm, 0);
	hGMHook = SetWindowsHookEx(WH_GETMESSAGE, proc2, hVdm, 0);
	PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
	return true;
}

bool DeInit()
{
	UnhookWindowsHookEx(hCWPHook);
	UnhookWindowsHookEx(hGMHook);
	PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
	FreeLibrary(hVdm);
	return true;
}