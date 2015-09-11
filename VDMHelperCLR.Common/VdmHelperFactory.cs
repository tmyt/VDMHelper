using System;
using System.IO;
using System.Reflection;

namespace VDMHelperCLR.Common
{
    public static class VdmHelperFactory
    {
        private static string DllName32 = "VDMHelperCLR32.dll";
        private static string DllName64 = "VDMHelperCLR64.dll";

        private static string GetPlatformDllName()
        {
            var dir = Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + "\\";
            return dir + (!Environment.Is64BitProcess ? DllName32 : DllName64);
        }

        public static IVdmHelper CreateInstance()
        {
            var asm = Assembly.LoadFile(GetPlatformDllName());
            var type = asm.GetType("VDMHelperCLR.VdmHelper");
            return (IVdmHelper)Activator.CreateInstance(type);
        }
    }
}
