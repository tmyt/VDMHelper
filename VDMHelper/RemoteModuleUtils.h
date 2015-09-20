#pragma once

#include <windows.h>
#include <psapi.h>

#define AddressOf(_type, _member) ((SIZE_T)(&(((_type*)((void*)0x0))->_member)))
#define RVA(_Base, _Offset) ((void*)(((SIZE_T)_Base) + _Offset))

#define CallOnRemoteThread(_process, _fn, _addr) \
	do{	\
		HANDLE _thread = ::CreateRemoteThread(_process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(_fn), _addr, 0, nullptr); \
		WaitForSingleObject(_thread, INFINITE); \
		CloseHandle(_thread); \
	}while(false)

bool GetRemoteModuleInfo(HANDLE hProcess, LPCTSTR lpModuleName, MODULEINFO* moduleInfo, bool is32bit);
void* GetRemoteProcAddress(HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName, bool is32bit);
void GetPreferredModuleName(HMODULE hModule, bool is32bit, TCHAR* szPath);
