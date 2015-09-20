// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "VDMHelper.h"
#include "VDMHelperAPI.h"

VDM::Helper g_helper;
HMODULE g_hModule;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hModule = hModule;
		g_helper.init();
		break;
	case DLL_PROCESS_DETACH:
		g_helper.deinit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}


LRESULT CALLBACK VDMHookProc1(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		auto cwp = reinterpret_cast<CWPRETSTRUCT*>(lParam);
		g_helper.process(cwp->hwnd, cwp->message, cwp->wParam, cwp->lParam);
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK VDMHookProc2(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && wParam == PM_REMOVE)
	{
		auto msg = reinterpret_cast<MSG*>(lParam);
		g_helper.process(msg->hwnd, msg->message, msg->wParam, msg->lParam);
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LPVOID CALLBACK VDMAllocGuid(HWND hwnd, const GUID* from)
{
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	auto hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
	auto remote = VirtualAllocEx(hProcess, nullptr, sizeof(GUID), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, remote, from, sizeof(GUID), nullptr);
	CloseHandle(hProcess);
	return remote;
}

LRESULT CALLBACK VDMReleaseGuid(HWND hwnd, LPVOID ptr)
{
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	auto hProcess = OpenProcess(PROCESS_VM_OPERATION, FALSE, pid);
	VirtualFreeEx(hProcess, ptr, sizeof(GUID), MEM_RELEASE);
	CloseHandle(hProcess);
	return 0;
}

LRESULT CALLBACK VDMProcess()
{
	TCHAR szSharedName[MAX_PATH];
	_stprintf_s(szSharedName, _T("VDM_Shared_%u"), GetCurrentProcessId());
	auto hMapping = OpenFileMapping(FILE_MAP_READ, FALSE, szSharedName);
	auto pView = reinterpret_cast<VDM::Shared*>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0));
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	IVirtualDesktopManager *pVdm;
	CoCreateInstance(CLSID_VirtualDesktopManager, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pVdm));
	pVdm->MoveWindowToDesktop(pView->hwnd, pView->guid);
	SetForegroundWindow(pView->hwnd);
	pVdm->Release();
	UnmapViewOfFile(pView);
	CloseHandle(hMapping);
	CoUninitialize();
	return 0;
}

LRESULT CALLBACK VDMInject(HWND hwnd, const GUID* from)
{
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(g_hModule, szPath, _countof(szPath));

	TCHAR szSharedName[MAX_PATH];
	_stprintf_s(szSharedName, _T("VDM_Shared_%u"), pid);
	auto hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
		0, sizeof(VDM::Shared), szSharedName);
	auto pView = reinterpret_cast<VDM::Shared*>(MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0));
	pView->hwnd = hwnd;
	memcpy(&pView->guid, from, sizeof(GUID));
	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	auto remote = VirtualAllocEx(hProcess, nullptr, _countof(szPath), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, remote, szPath, _countof(szPath), nullptr);
	auto hThread = CreateRemoteThread(hProcess, nullptr, 0,
		reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "LoadLibraryW")),
		remote, 0, nullptr);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	HMODULE hModules[100];
	DWORD size;
	EnumProcessModules(hProcess, hModules, sizeof(hModules), &size);
	TCHAR szModuleName[MAX_PATH];
	MODULEINFO moduleInfo;
	for (auto i = 0; i < _countof(hModules); ++i)
	{
		GetModuleBaseName(hProcess, hModules[i], szModuleName, _countof(szModuleName));
		GetModuleInformation(hProcess, hModules[i], &moduleInfo, sizeof(moduleInfo));
		if (_tcscmp(_T("VDMHelper32.dll"), szModuleName) == 0)
		{
			break;
		}
	}
	auto address = reinterpret_cast<uintptr_t>(&VDMProcess) - reinterpret_cast<uintptr_t>(g_hModule);
	auto proc = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll) + address;
	hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(proc), nullptr, 0, nullptr);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	hThread = CreateRemoteThread(hProcess, nullptr, 0,
		reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "FreeLibrary")),
		moduleInfo.lpBaseOfDll, 0, nullptr);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	UnmapViewOfFile(pView);
	CloseHandle(hFileMapping);
	VirtualFreeEx(hProcess, remote, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return 0;
}