// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "VDMHelper.h"
#include "VDMHelperAPI.h"

#include "RemoteModuleUtils.h"

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

LRESULT CALLBACK VDMProcess(VDM::SharedValue* pView)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	IVirtualDesktopManager *pVdm;
	CoCreateInstance(CLSID_VirtualDesktopManager, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pVdm));
	pVdm->MoveWindowToDesktop((HWND)pView->hwnd, pView->guid);
	SetForegroundWindow((HWND)pView->hwnd);
	pVdm->Release();
	CoUninitialize();
	return 0;
}

void VDMInject32(HANDLE hProcess, unsigned int hwnd, const GUID* from)
{
	TCHAR szPath[MAX_PATH] = {0};
	GetPreferredModuleName(g_hModule, true, szPath);
	
	// construct arguments
	auto shared = VirtualAllocEx(hProcess, nullptr, sizeof(VDM::Shared32), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, RVA(shared, AddressOf(VDM::Shared32, hwnd)), &hwnd, sizeof(HWND), nullptr);
	WriteProcessMemory(hProcess, RVA(shared, AddressOf(VDM::Shared32, guid)), from, sizeof(*from), nullptr);

	MODULEINFO kernel32;
	GetRemoteModuleInfo(hProcess, _T("kernel32.dll"), &kernel32, true);
	auto fnLoadLibrary = GetRemoteProcAddress(hProcess, (HMODULE)kernel32.lpBaseOfDll, "LoadLibraryW", true);
	auto remote = VirtualAllocEx(hProcess, nullptr, _countof(szPath), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, remote, szPath, _countof(szPath), nullptr);
	CallOnRemoteThread(hProcess, fnLoadLibrary, remote);

	MODULEINFO vdmhelper;
	GetRemoteModuleInfo(hProcess, _T("VDMHelper32.dll"), &vdmhelper, true);
	auto fnVDMProcess = GetRemoteProcAddress(hProcess, (HMODULE)vdmhelper.lpBaseOfDll, "VDMProcess", true);
	CallOnRemoteThread(hProcess, fnVDMProcess, shared);

	auto fnFreeLibrary = GetRemoteProcAddress(hProcess, (HMODULE)kernel32.lpBaseOfDll, "FreeLibrary", true);
	CallOnRemoteThread(hProcess, fnFreeLibrary, vdmhelper.lpBaseOfDll);

	VirtualFreeEx(hProcess, remote, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, shared, 0, MEM_RELEASE);
}

void VDMInject64(HANDLE hProcess, HWND hwnd, const GUID* from)
{
	TCHAR szPath[MAX_PATH] = { 0 };
	GetPreferredModuleName(g_hModule, false, szPath);

	// construct arguments
	auto shared = VirtualAllocEx(hProcess, nullptr, sizeof(VDM::Shared64), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, RVA(shared, AddressOf(VDM::Shared64, hwnd)), &hwnd, sizeof(HWND), nullptr);
	WriteProcessMemory(hProcess, RVA(shared, AddressOf(VDM::Shared64, guid)), from, sizeof(*from), nullptr);

	MODULEINFO kernel32;
	GetRemoteModuleInfo(hProcess, _T("kernel32.dll"), &kernel32, false);
	auto fnLoadLibrary = GetRemoteProcAddress(hProcess, (HMODULE)kernel32.lpBaseOfDll, "LoadLibraryW", false);
	auto remote = VirtualAllocEx(hProcess, nullptr, _countof(szPath), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, remote, szPath, _countof(szPath), nullptr);
	CallOnRemoteThread(hProcess, fnLoadLibrary, remote);

	MODULEINFO vdmhelper;
	GetRemoteModuleInfo(hProcess, _T("VDMHelper64.dll"), &vdmhelper, false);
	auto fnVDMProcess = GetRemoteProcAddress(hProcess, (HMODULE)vdmhelper.lpBaseOfDll, "VDMProcess", false);
	CallOnRemoteThread(hProcess, fnVDMProcess, shared);

	auto fnFreeLibrary = GetRemoteProcAddress(hProcess, (HMODULE)kernel32.lpBaseOfDll, "FreeLibrary", false);
	CallOnRemoteThread(hProcess, fnFreeLibrary, vdmhelper.lpBaseOfDll);

	VirtualFreeEx(hProcess, remote, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, shared, 0, MEM_RELEASE);
}

LRESULT CALLBACK VDMInject(HWND hwnd, const GUID* from)
{
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);

	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

#ifdef _WIN64
	BOOL isWow64;
	IsWow64Process(hProcess, &isWow64);

	if(!isWow64)
	{
		VDMInject64(hProcess, hwnd, from);
	}else
#endif
	{
		VDMInject32(hProcess, (unsigned int)hwnd, from);
	}

	CloseHandle(hProcess);
	return 0;
}