#include "Common.h"
#include "ServerDriver.h" 
#include "WatchDogDriver.h" 

void CreateDefaultSettings(HINSTANCE hm);

CServerDriver g_ServerDriver;
CWatchDogDriver g_WatchDogDriver;

HMODULE g_hModule = nullptr;

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			g_hModule = hinstDLL;
			CreateDefaultSettings(hinstDLL);

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
			_snprintf_s(buff, sizeof(buff), "%s..\\..\\resources\\%s", DllPath, platform);
			SetDllDirectoryA(buff);
		}
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

HMD_DLL_EXPORT void* HmdDriverFactory(const char* interface_name, int* return_code)
{	
	if (return_code) {
		*return_code = vr::VRInitError_None;
	}
	
	if (0 == std::strcmp(vr::IServerTrackedDeviceProvider_Version, interface_name)) {
		return (IServerTrackedDeviceProvider *) &g_ServerDriver;
	}

	if (0 == std::strcmp(vr::IVRWatchdogProvider_Version, interface_name)) {
		return (IVRWatchdogProvider *) &g_WatchDogDriver;
	}

	if (return_code) {
		*return_code = vr::VRInitError_Init_InterfaceNotFound;
	}

	return nullptr;
}

void CreateDefaultSettings(HINSTANCE hm)
{
	char dllPath[MAX_PATH];
	//HMODULE hm = nullptr;
	//if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&CreateDefaultSettings, &hm))
		//return;
	GetModuleFileNameA(hm, dllPath, sizeof(dllPath));
	PathRemoveFileSpecA(dllPath);


	struct stat buffer;
	FILE *fp = nullptr;

	char filename[MAX_PATH];
	sprintf_s(filename, "%s\\..\\..\\resources\\settings\\default.vrsettings", dllPath);
		
	if (stat(filename, &buffer) != 0)
	{

		sprintf_s(filename, "%s\\..\\..\\resources", dllPath);
		CreateDirectoryA(filename, nullptr);
		sprintf_s(filename, "%s\\..\\..\\resources\\settings", dllPath);
		CreateDirectoryA(filename, nullptr);
		sprintf_s(filename, "%s\\..\\..\\resources\\settings\\default.vrsettings", dllPath);
				
		fopen_s(&fp, filename, "w");
		if (fp != nullptr)
		{
			//ipd adjustment will be moved to HMD ipd knob
			fprintf_s(fp, "{\"driver_customhmd\": {\"IPD\" : 0.05, \"camera\":\"Webcam C170\", \"monitor\":\"SNYD602\", \"supersample\":1.0 }}");
			fclose(fp);
			fp = nullptr;
		}
	}

	sprintf_s(filename, "%s\\..\\..\\driver.vrdrivermanifest", dllPath);
	if (stat(filename, &buffer) != 0)
	{
		fopen_s(&fp, filename, "w");
		if (fp != nullptr)
		{
			fprintf_s(fp, "{\"alwaysActivate\": true, \"name\" : \"customhmd\", \"directory\" : \"\", \"resourceOnly\" : false, \"hmd_presence\" : [ \"1974.0001\" ] }");
			fclose(fp);
			fp = nullptr;
		}
	}


}

