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
	if (pMonData->Logger) pMonData->Logger->Log("Enumerate callback\n");

	MONITORINFOEX monInfo = {};
	monInfo.cbSize = sizeof(monInfo);
	wchar_t DeviceID[4096] = {};
	if (GetMonitorInfo(hMonitor, &monInfo))
	{
		DISPLAY_DEVICE ddMon;
		ZeroMemory(&ddMon, sizeof(ddMon));
		ddMon.cb = sizeof(ddMon);
		DWORD devMon = 0;
		if (pMonData->Logger) pMonData->Logger->Log("Enumerating monitors for:\n");
		if (pMonData->Logger) pMonData->Logger->Log(monInfo.szDevice);		
		while (EnumDisplayDevices(monInfo.szDevice, devMon, &ddMon, 0))
		{			
			if (pMonData->Logger) pMonData->Logger->Log(ddMon.DeviceID);
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

							if (pMonData->Logger) pMonData->Logger->Log("\nFOUND MONITOR\n");
							return FALSE;
						}
					}
				}
			}
			devMon++;

			ZeroMemory(&ddMon, sizeof(ddMon));
			ddMon.cb = sizeof(ddMon);		
		}
		if (pMonData->Logger) pMonData->Logger->Log("No more monitors.\n");
	}
	return TRUE;
}

vr::HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z)
{
	vr::HmdQuaternion_t quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	return quat;
}

void HmdMatrix_SetIdentity(vr::HmdMatrix34_t *pMatrix)
{
	pMatrix->m[0][0] = 1.f;
	pMatrix->m[0][1] = 0.f;
	pMatrix->m[0][2] = 0.f;
	pMatrix->m[0][3] = 0.f;
	pMatrix->m[1][0] = 0.f;
	pMatrix->m[1][1] = 1.f;
	pMatrix->m[1][2] = 0.f;
	pMatrix->m[1][3] = 0.f;
	pMatrix->m[2][0] = 0.f;
	pMatrix->m[2][1] = 0.f;
	pMatrix->m[2][2] = 1.f;
	pMatrix->m[2][3] = 0.f;
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