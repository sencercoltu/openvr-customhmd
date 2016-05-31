#include "Common.h"
#include "ServerDriver.h" 
#include "ClientDriver.h" 


CServerDriver g_ServerDriver;
CClientDriver g_ClientDriver;


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
	auto pMonData = (HMDData *)dwData;
	if (pMonData->Logger) pMonData->Logger->Log("Monitor Enumeration callback..");

	MONITORINFOEX monInfo = {};
	monInfo.cbSize = sizeof(monInfo);
	wchar_t DeviceID[4096] = {};
	if (GetMonitorInfo(hMonitor, &monInfo))
	{
		DISPLAY_DEVICE ddMon;
		ZeroMemory(&ddMon, sizeof(ddMon));
		ddMon.cb = sizeof(ddMon);
		DWORD devMon = 0;
		if (pMonData->Logger) pMonData->Logger->Log("Enumerating monitors for %S...", monInfo.szDevice);		
		while (EnumDisplayDevices(monInfo.szDevice, devMon, &ddMon, 0))
		{			
			if (pMonData->Logger) pMonData->Logger->Log("Checking %S...", ddMon.DeviceID);
			if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE && !(ddMon.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
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
						if (!wcscmp(pStart, pMonData->Model)) //look for this monitor id (SonyHMZ-T2)
						{							
							wcscpy_s(pMonData->DisplayName, monInfo.szDevice);
							pMonData->PosX = monInfo.rcMonitor.left;
							pMonData->PosY = monInfo.rcMonitor.top;
							pMonData->ScreenWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
							pMonData->ScreenHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;							
							if (pMonData->ScreenWidth == 1280 && pMonData->ScreenHeight == 1470)
							{
								pMonData->FakePackDetected = true;
								pMonData->AspectRatio = ((float)(pMonData->ScreenHeight - 30) / 2.0f) / (float)pMonData->ScreenWidth;
							}
							else
							{
								pMonData->FakePackDetected = false;
								pMonData->AspectRatio = (float)pMonData->ScreenWidth / (float)pMonData->ScreenHeight;
							}
							DEVMODE devMode = {};
							devMode.dmSize = sizeof(DEVMODE);
							if (EnumDisplaySettings(monInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
								pMonData->Frequency = (float)devMode.dmDisplayFrequency;
							pMonData->IsConnected = true;

							if (pMonData->Logger) pMonData->Logger->Log("Found monitor %S.", pMonData->DisplayName);
							return FALSE;
						}
					}
				}
			}
			devMon++;

			ZeroMemory(&ddMon, sizeof(ddMon));
			ddMon.cb = sizeof(ddMon);		
		}
		if (pMonData->Logger) pMonData->Logger->Log("No more monitors!");
	}
	return TRUE;
}

void CDummyLog::Log(const char* pMsg)
{
	FILE *fp = nullptr;
	fopen_s(&fp, "D:\\hmd.log", "at");
	if (fp)
	{
		fprintf(fp, "%s", pMsg);
		fclose(fp);
		fp = nullptr;
	}
}
