#include "Common.h"
#include "ServerDriver.h" 
#include "ClientDriver.h" 


static CServerDriver g_ServerDriver;
static CClientDriver g_ClientDriver;


HMD_DLL_EXPORT void* HmdDriverFactory(const char* interface_name, int* return_code)
{
	if (return_code) {
		*return_code = vr::VRInitError_None;
	}

	if (0 == std::strcmp(vr::IServerTrackedDeviceProvider_Version, interface_name)) {
		return &g_ServerDriver;
	}

	if (0 == std::strcmp(vr::IClientTrackedDeviceProvider_Version, interface_name)) {
		return &g_ClientDriver;
	}

	if (return_code) {
		*return_code = vr::VRInitError_Init_InterfaceNotFound;
	}

	return NULL;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX monInfo = {};
	monInfo.cbSize = sizeof(monInfo);
	wchar_t DeviceID[4096] = {};
	if (GetMonitorInfo(hMonitor, &monInfo))
	{
		DISPLAY_DEVICE ddMon;
		ZeroMemory(&ddMon, sizeof(ddMon));
		ddMon.cb = sizeof(ddMon);
		DWORD devMon = 0;

		while (EnumDisplayDevices(monInfo.szDevice, devMon, &ddMon, 0))
		{
			if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE &&
				!(ddMon.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
			{
				wsprintf(DeviceID, L"%s", ddMon.DeviceID);
				wchar_t *pStart = wcschr(DeviceID, L'\\');
				if (pStart)
				{
					pStart++;
					wchar_t *pEnd = wcschr(pStart, L'\\');
					if (pEnd)
					{
						*pEnd = 0;
						if (!wcscmp(pStart, L"SNYD602")) //look for this monitor id (SonyHMZ-T2)
						{
							auto pMonData = (MonitorData *)dwData;
							pMonData->HMD_POSX = monInfo.rcMonitor.left;
							pMonData->HMD_POSY = monInfo.rcMonitor.top;
							pMonData->HMD_WIDTH = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
							pMonData->HMD_HEIGHT = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
							pMonData->HMD_ASPECT = (float)pMonData->HMD_WIDTH / (float)pMonData->HMD_HEIGHT;
							DEVMODE devMode = {};
							devMode.dmSize = sizeof(DEVMODE);
							if (EnumDisplaySettings(monInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
								pMonData->HMD_FREQ = (float)devMode.dmDisplayFrequency;
							pMonData->HMD_FOUND = true;
							return FALSE;
						}
					}
				}
			}
			devMon++;

			ZeroMemory(&ddMon, sizeof(ddMon));
			ddMon.cb = sizeof(ddMon);
		}
	}
	return TRUE;
}
