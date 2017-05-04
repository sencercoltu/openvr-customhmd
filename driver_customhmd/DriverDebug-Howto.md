# How to debug the OpenVR driver

* Use Visual Studio
* Compile the driver, for both x86 and x64 platforms.
* Open a command prompt, go to your drivers bin folder, create a hardlink to Debug/driver_customhmd.dll int both platform folders.
  * mklink /h Win32\driver_customhmd.dll "_path_to_project"\x86\Debug\driver_customhmd.dll
  * mklink /h Win64\driver_customhmd.dll "_path_to_project"\x64\Debug\driver_customhmd.dll
* Set debug executable to Openvr's vrserver.exe (64 bits), and start to debug while SteamVR is closed. The debugger will wait for the dll to be loaded.
* Start SteamVR, now you should be able to debug. If no OpenVR application is launched in a certain time, vrserver.exe will shutdown.
* Or if you don't need to debug initialization steps, just attach the debugger to an existing vrserver.exe
