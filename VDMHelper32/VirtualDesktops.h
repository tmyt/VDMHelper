#pragma once

#include "stdafx.h"
#include <objbase.h>
#include <ObjectArray.h>

const CLSID CLSID_ImmersiveShell = {
	0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };

const CLSID CLSID_VirtualDesktopAPI_Unknown = {
	0xC5E0CDCA, 0x7B6E, 0x41B2, 0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B };

const IID IID_IVirtualDesktopManagerInternal = {
	0xEF9F1A6C, 0xD3CC, 0x4358, 0xB7, 0x12, 0xF8, 0x4B, 0x63, 0x5B, 0xEB, 0xE7 };

// ÑÉÑ}. IApplicationView ÑyÑx Windows Runtime
struct IApplicationView : public IUnknown
{
public:

};

// ÑBÑyÑÇÑÑÑÖÑpÑ|ÑéÑ~ÑçÑz ÑÉÑÑÑÄÑ|

EXTERN_C const IID IID_IVirtualDesktop;

MIDL_INTERFACE("FF72FFDD-BE7E-43FC-9C03-AD81681E88E4")
IVirtualDesktop : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsViewVisible(
		IApplicationView *pView,
		int *pfVisible) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetID(
		GUID *pGuid) = 0;
};

enum AdjacentDesktop
{
	// ÑRÑÄÑÉÑuÑtÑ~ÑyÑz ÑÇÑpÑqÑÄÑâÑyÑz ÑÉÑÑÑÄÑ| ÑÉÑ|ÑuÑrÑp
	LeftDirection = 3,
	// ÑRÑÄÑÉÑuÑtÑ~ÑyÑz ÑÇÑpÑqÑÄÑâÑyÑz ÑÉÑÑÑÄÑ| ÑÉÑÅÑÇÑpÑrÑp
	RightDirection = 4
};

// ÑMÑuÑ~ÑuÑtÑwÑuÑÇ ÑrÑyÑÇÑÑÑÖÑpÑ|ÑéÑ~ÑçÑá ÑÉÑÑÑÄÑ|ÑÄÑr

EXTERN_C const IID IID_IVirtualDesktopManagerInternal;

// 10130
//MIDL_INTERFACE("EF9F1A6C-D3CC-4358-B712-F84B635BEBE7")
// 10240
MIDL_INTERFACE("AF8DA486-95BB-4460-B3B7-6E7A6B2962B5")
IVirtualDesktopManagerInternal : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetCount(
		UINT *pCount) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop(
		IApplicationView *pView,
		IVirtualDesktop *pDesktop) = 0;

	// 10240
	virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops(
		IApplicationView *pView,
		int *pfCanViewMoveDesktops) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(
		IVirtualDesktop** desktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetDesktops(
		IObjectArray **ppDesktops) = 0;

	// ÑPÑÄÑ|ÑÖÑâÑuÑ~ÑyÑu ÑÉÑÄÑÉÑuÑtÑ~ÑuÑsÑÄ ÑÇÑpÑqÑÄÑâÑuÑsÑÄ ÑÉÑÑÑÄÑ|Ñp ÑÄÑÑÑ~ÑÄÑÉÑyÑÑÑuÑ|ÑéÑ~ÑÄ ÑÖÑ{ÑpÑxÑpÑ~Ñ~ÑÄÑsÑÄ, ÑÉ ÑÖÑâÑuÑÑÑÄÑ} Ñ~ÑpÑÅÑÇÑpÑrÑ|ÑuÑ~ÑyÑë
	virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop(
		IVirtualDesktop *pDesktopReference,
		AdjacentDesktop uDirection,
		IVirtualDesktop **ppAdjacentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(
		IVirtualDesktop *pDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateDesktopW(
		IVirtualDesktop **ppNewDesktop) = 0;

	// pFallbackDesktop - ÑÇÑpÑqÑÄÑâÑyÑz ÑÉÑÑÑÄÑ| Ñ~Ñp Ñ{ÑÄÑÑÑÄÑÇÑçÑz ÑqÑÖÑtÑuÑÑ ÑÉÑÄÑrÑuÑÇÑäÑuÑ~ ÑÅÑuÑÇÑuÑáÑÄÑt ÑÅÑÄÑÉÑ|Ñu ÑÖÑtÑpÑ|ÑuÑ~ÑyÑë ÑÖÑ{ÑpÑxÑpÑ~Ñ~ÑÄÑsÑÄ
	virtual HRESULT STDMETHODCALLTYPE RemoveDesktop(
		IVirtualDesktop *pRemove,
		IVirtualDesktop *pFallbackDesktop) = 0;

	// 10240
	virtual HRESULT STDMETHODCALLTYPE FindDesktop(
		GUID *desktopId,
		IVirtualDesktop **ppDesktop) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopManager;

MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out BOOL *onCurrentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out GUID *desktopId) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [in] */ __RPC__in REFGUID desktopId) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopNotification;

MIDL_INTERFACE("C179334C-4295-40D3-BEA1-C654D965605A")
IVirtualDesktopNotification : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(
		IVirtualDesktop *pDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop *pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop *pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop *pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(
		IApplicationView *pView) = 0;

	virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(
		IVirtualDesktop *pDesktopOld,
		IVirtualDesktop *pDesktopNew) = 0;

};

EXTERN_C const IID IID_IVirtualDesktopNotificationService;

MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
IVirtualDesktopNotificationService : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Register(
		IVirtualDesktopNotification *pNotification,
		DWORD *pdwCookie) = 0;

	virtual HRESULT STDMETHODCALLTYPE Unregister(
		DWORD dwCookie) = 0;
};
