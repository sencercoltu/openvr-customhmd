#include "ClientDriver.h"

EVRInitError CClientDriver::Init(IDriverLog * pDriverLog, IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{
	//MessageBox(NULL, L"CClientDriver::Init.", L"Info", 0);
	//TRACE(__FUNCTIONW__);
	logger_ = pDriverLog;
	driverHost_ = pDriverHost;
	userDriverConfigDir_ = pchUserDriverConfigDir;
	driverInstallDir_ = pchDriverInstallDir;

	if (pchUserDriverConfigDir) logger_->Log(pchUserDriverConfigDir);
	if (pchDriverInstallDir) logger_->Log(pchDriverInstallDir);


	if (!m_bInit)
	{
		if (driverHost_)
		{
			IVRSettings *pSettings = driverHost_->GetSettings(vr::IVRSettings_Version);
		}
		m_bInit = true;
	}	
	//MessageBox(NULL, L"Clientdriver init", L"Info", 0);
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
	CDummyLog g_DuymmyLog;
	//HMDLog tLog(&g_DuymmyLog);	
	HMDData hmdData = { 0 };
	wcscpy_s(hmdData.Model, L"SNYD602");
	//hmdData.Logger = &tLog;
	//if (hmdData.Logger && pchUserConfigDir) hmdData.Logger->Log(pchUserConfigDir);
	//if (hmdData.Logger) hmdData.Logger->Log("Enumnerating monitors...\n");
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&hmdData);
	return hmdData.IsConnected;
}

EVRInitError CClientDriver::SetDisplayId(const char * pchDisplayId)
{
	//TRACE(__FUNCTIONW__);	
	//EnableFakePack();		
	//MessageBox(NULL, L"Displayid set", wstr.c_str(), 0);	
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
