#include "ClientDriver.h"

EVRInitError CClientDriver::Init(IDriverLog * pDriverLog, IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{
	//MessageBox(NULL, L"CClientDriver::Init.", L"Info", 0);
	//TRACE(__FUNCTIONW__);
	logger_ = pDriverLog;
	driverHost_ = pDriverHost;
	userDriverConfigDir_ = pchUserDriverConfigDir;
	driverInstallDir_ = pchDriverInstallDir;
	

//#ifdef HMD_MODE_FAKEPACK	
//	MonitorData m_MonData = {};
//	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&m_MonData);
//
//	//	TRACE(__FUNCTIONW__);
//	DEVMODE displayMode = {};
//	displayMode.dmSize = sizeof(DEVMODE);
//	EnumDisplaySettings(m_MonData.DisplayName, ENUM_CURRENT_SETTINGS, &displayMode);
//
//	if (displayMode.dmPelsWidth != 1280 || displayMode.dmPelsHeight != 1470)
//	{
//		displayMode.dmPelsWidth = 1280;
//		displayMode.dmPelsHeight = 1470;
//		displayMode.dmBitsPerPel = 32;
//		displayMode.dmDisplayFrequency = 60;
//		int ChangeDisplayResult = ChangeDisplaySettingsEx(m_MonData.DisplayName, &displayMode, nullptr, CDS_FULLSCREEN, nullptr);
//		if (ChangeDisplayResult != DISP_CHANGE_SUCCESSFUL)
//		{
//			MessageBox(NULL, L"Error: Failed to change display mode.", L"Error", 0);
//			return vr::VRInitError_VendorSpecific_HmdFound_CantOpenDevice;
//		}
//	}
//	//tekrar al yeni halini
//	//EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&m_MonData);
//#endif //HMD_MODE_FAKEPACK

	if (!m_bInit)
	{
		if (driverHost_)
		{
			IVRSettings *pSettings = driverHost_->GetSettings(vr::IVRSettings_Version);
		}
		m_bInit = true;
	}
	EnableFakePack();
	return vr::VRInitError_None;
}

void CClientDriver::Cleanup()
{
	//TRACE(__FUNCTIONW__);
	logger_ = nullptr;
	driverHost_ = nullptr;
	userDriverConfigDir_.clear();
	driverInstallDir_.clear();
}

bool CClientDriver::BIsHmdPresent(const char * pchUserConfigDir)
{
	//TRACE(__FUNCTIONW__);
	MonitorData monData = { 0 };
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&monData);
	return monData.HMD_FOUND;
}

EVRInitError CClientDriver::SetDisplayId(const char * pchDisplayId)
{
	//TRACE(__FUNCTIONW__);	
	return vr::VRInitError_None;
}

HiddenAreaMesh_t CClientDriver::GetHiddenAreaMesh(EVREye eEye)
{
	//TRACE(__FUNCTIONW__);
	vr::HiddenAreaMesh_t hidden_area_mesh;
	hidden_area_mesh.pVertexData = nullptr;
	hidden_area_mesh.unTriangleCount = 0;
	return hidden_area_mesh;
}

uint32_t CClientDriver::GetMCImage(uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen)
{
	//TRACE(__FUNCTIONW__);
	return 0;
}
