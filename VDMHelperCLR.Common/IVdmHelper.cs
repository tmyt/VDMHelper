using System;

namespace VDMHelperCLR.Common
{
    public interface IVdmHelper : IDisposable
    {
        bool Init();
        bool DeInit();
        bool MoveWindowToDesktop(IntPtr topLevelWindow, Guid desktopId);
    }
}
