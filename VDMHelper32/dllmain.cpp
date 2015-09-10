// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "VDMHelper.h"
#include "VDMHelperAPI.h"

VDM::Helper g_helper;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
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