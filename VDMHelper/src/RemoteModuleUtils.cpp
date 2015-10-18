#include "stdafx.h"

#include <Windows.h>
#include <psapi.h>
#include <tchar.h>

#include "RemoteModuleUtils.h"

#define GetRemoteDosHeader GetProcessStruct
#define GetNtHeader GetProcessStruct
#define GetExportDirectory GetProcessStruct
#define GetRvaArray GetProcessStructArray

template<typename T>
BOOL GetProcessStruct(HANDLE hProcess, void* hModule, T* pHdr)
{
	SIZE_T t;
	ReadProcessMemory(hProcess, hModule, pHdr, sizeof(T), &t);
	return TRUE;
}

template<typename T>
BOOL GetProcessStructArray(HANDLE hProcess, void* hModule, T* pHdr, int length)
{
	SIZE_T t;
	ReadProcessMemory(hProcess, hModule, pHdr, sizeof(T) * length, &t);
	return TRUE;
}

int GetProcIndex(HANDLE hProcess, HMODULE hModule, IMAGE_EXPORT_DIRECTORY* eat, LPCSTR lpProcName)
{
	DWORD* nameRvas = new DWORD[eat->NumberOfNames];
	GetRvaArray(hProcess, RVA(hModule, eat->AddressOfNames), nameRvas, eat->NumberOfNames);

	auto l = eat->NumberOfNames;
	int b = 0, e = l;
	int ordinal = -1;
	while (l != 0)
	{
		char name[128];
		auto i = (e - b) >> 1;
		auto p = b + i;
		GetRvaArray(hProcess, RVA(hModule, nameRvas[p]), name, 120);
		auto cmp = strcmp(lpProcName, name);
		if (cmp == 0) {
			ordinal = p;
			break;
		}
		if (cmp > 0)
			b = p;
		else
			e = p;
		if (i == 0) break;
	}
	delete[] nameRvas;
	return ordinal;
}

bool GetRemoteModuleInfo(HANDLE hProcess, LPCTSTR lpModuleName, MODULEINFO* moduleInfo, bool is32bit)
{
	HMODULE hModules[100];
	DWORD size;
	EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &size, is32bit ? LIST_MODULES_32BIT : LIST_MODULES_64BIT);
	TCHAR szModuleName[MAX_PATH];
	for (auto i = 0; i < _countof(hModules); ++i)
	{
		GetModuleBaseName(hProcess, hModules[i], szModuleName, _countof(szModuleName));
		GetModuleInformation(hProcess, hModules[i], moduleInfo, sizeof(MODULEINFO));
		if (_tcsicmp(lpModuleName, szModuleName) == 0)
		{
			return true;
		}
	}
	return false;
}

void* GetRemoteProcAddress32(HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName)
{
	IMAGE_DOS_HEADER hdr;
	IMAGE_NT_HEADERS32 nthdr;
	IMAGE_EXPORT_DIRECTORY eat;

	GetRemoteDosHeader(hProcess, hModule, &hdr);
	GetNtHeader(hProcess, RVA(hModule, hdr.e_lfanew), &nthdr);
	auto exportDirectory = nthdr.OptionalHeader.DataDirectory[0];
	GetExportDirectory(hProcess, RVA(hModule, exportDirectory.VirtualAddress), &eat);
	auto ordinal = GetProcIndex(hProcess, hModule, &eat, lpProcName);

	WORD* ordinals = new WORD[eat.NumberOfNames];
	GetRvaArray(hProcess, RVA(hModule, eat.AddressOfNameOrdinals), ordinals, eat.NumberOfNames);

	DWORD* functions = new DWORD[eat.NumberOfFunctions];
	GetRvaArray(hProcess, RVA(hModule, eat.AddressOfFunctions), functions, eat.NumberOfFunctions);

	auto ptr = functions[ordinals[ordinal]];

	delete[] functions;
	delete[] ordinals;
	return RVA(hModule, ptr);
}

void* GetRemoteProcAddress64(HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName)
{
	IMAGE_DOS_HEADER hdr;
	IMAGE_NT_HEADERS64 nthdr;
	IMAGE_EXPORT_DIRECTORY eat;

	GetRemoteDosHeader(hProcess, hModule, &hdr);
	GetNtHeader(hProcess, RVA(hModule, hdr.e_lfanew), &nthdr);
	auto exportDirectory = nthdr.OptionalHeader.DataDirectory[0];
	GetExportDirectory(hProcess, RVA(hModule, exportDirectory.VirtualAddress), &eat);
	auto ordinal = GetProcIndex(hProcess, hModule, &eat, lpProcName);

	WORD* ordinals = new WORD[eat.NumberOfNames];
	GetRvaArray(hProcess, RVA(hModule, eat.AddressOfNameOrdinals), ordinals, eat.NumberOfNames);

	DWORD* functions = new DWORD[eat.NumberOfFunctions];
	GetRvaArray(hProcess, RVA(hModule, eat.AddressOfFunctions), functions, eat.NumberOfFunctions);

	auto ptr = functions[ordinals[ordinal]];

	delete[] functions;
	delete[] ordinals;
	return RVA(hModule, ptr);
}

void* GetRemoteProcAddress(HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName, bool is32bit)
{
	return is32bit ? GetRemoteProcAddress32(hProcess, hModule, lpProcName) : GetRemoteProcAddress64(hProcess, hModule, lpProcName);
}

void GetPreferredModuleName(HMODULE hModule, bool is32bit, TCHAR* szPath)
{
	GetModuleFileName(hModule, szPath, MAX_PATH);
	for (int i = MAX_PATH - 1; i >= 0; --i)
	{
		if (szPath[i] == '\\')
		{
			szPath[i + 1] = 0;
			break;
		}
	}
	_tcscat_s(szPath, MAX_PATH, is32bit ? _T("VDMHelper32.dll") : _T("VDMHelper64.dll"));
}
