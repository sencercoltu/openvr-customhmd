#include "Common.h"
#include <delayimp.h>

extern HMODULE g_hModule;

FARPROC WINAPI delayHookNotifyFunc(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliNotePreLoadLibrary:
		char DllPath[MAX_PATH] = { 0 };
		GetModuleFileNameA(g_hModule, DllPath, _countof(DllPath));
		//remove file
		while (DllPath[strlen(DllPath) - 1] != '\\')
			DllPath[strlen(DllPath) - 1] = 0;

		char *platform =
#ifdef _WIN64
			"win64";
#else
			"win32";
#endif
		char buff[1024];
		_snprintf_s(buff, sizeof(buff), "%s..\\..\\resources\\%s\\%s", DllPath, platform, pdli->szDll);


#ifdef _WIN64
		std::string msg = buff;
		std::wstring wmsg(msg.begin(), msg.end());
		return (FARPROC) LoadLibrary(wmsg.c_str());
#else
		return (FARPROC) LoadLibrary(buff);
#endif		
		break;
	}

	return nullptr;
}


FARPROC WINAPI delayHookFailureFunc(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliFailLoadLib:
		char buff[1024];
		_snprintf_s(buff, sizeof(buff), "Error loading %s, exiting.", pdli->szDll);
		std::string msg = buff;
		std::wstring wmsg(msg.begin(), msg.end());
		MessageBox(nullptr, wmsg.c_str(), L"XBMC: Fatal Error", MB_OK | MB_ICONERROR);		
		break;
	}
	return nullptr;
}

PfnDliHook __pfnDliNotifyHook2 = delayHookNotifyFunc;
PfnDliHook __pfnDliFailureHook2 = delayHookFailureFunc;

