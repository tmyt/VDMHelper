using System;

namespace VDMHelperCLR.Common
{
    public interface IVdmHelper : IDisposable
    {
        bool Init();
        bool DeInit();
        void MoveWindowToDesktop(IntPtr topLevelWindow, Guid desktopId);
    }
}
