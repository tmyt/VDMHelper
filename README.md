VirtualDesktopManagerHelper
====

how to use
----
Step1. Copy these files to same directory to your main exe file.

- VDMHelper32.dll
- VDMHelper64.dll
- VDMHelperCLR32.dll
- VDMHelperCLR64.dll
- VDMHelperCLR.Common.dll
- InjectDll32.exe (this is proxy for hookin 32bit process from 64bit process)

Step2. Add reference VDMHelperCLR.Common.dll to your project.

Step3. Run this code to start hook.
```cs
var dir = Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + "\\";
var is64Bit = Marshal.SizeOf(typeof(IntPtr)) == 8;
var asm = Assembly.LoadFile(dir + (is64Bit ? @"VDMHelperCLR64.dll" : @"VDMHelperCLR32.dll"));
var type = asm.GetType("VDMHelperCLR.VdmHelper");
this.helper = (IVdmHelper)Activator.CreateInstance(type);
this.helper.Init();
```

Step4. Stop hook when app closing.
```cs
this.helper.Dispose();
```

note
----

- If you use .NET 4.5 or higher, you should turn off `Prefer 32bit` flag on Build property.