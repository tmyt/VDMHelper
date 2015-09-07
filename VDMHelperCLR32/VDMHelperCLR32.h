// VDMHelperCLR32.h

#pragma once

#include "../VDMHelper32/VDMHelperAPI.h"


using namespace System;
using namespace VDMHelperCLR::Common;

namespace VDMHelperCLR {

	public ref class VdmHelper : public IVdmHelper
	{
	public:
		VdmHelper();
		~VdmHelper();

		virtual bool Init();
		virtual bool DeInit();
		virtual void MoveWindowToDesktop(IntPtr topLevelWindow, Guid desktopId);

	private:
		UINT RequestMoveWindowToDesktopMessage;

		HMODULE hvdm;
		decltype(::VDMHookProc)* VDMHookProc;
		decltype(::VDMAllocGuid)* VDMAllocGuid;
		decltype(::VDMReleaseGuid)* VDMReleaseGuid;

		HHOOK hHook;
	};
}
