#include "ClientDriver.h"
#include "hidapi.h"

EVRInitError CClientDriver::Init(IDriverLog * pDriverLog, IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{
	logger_ = pDriverLog;
	logger_->Log(__FUNCTION__"\n");	
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
	return vr::VRInitError_None;
}

void CClientDriver::Cleanup()
{
	logger_->Log(__FUNCTION__"\n");	logger_ = nullptr;
	driverHost_ = nullptr;
	userDriverConfigDir_.clear();
	driverInstallDir_.clear(); 
}

bool CClientDriver::BIsHmdPresent(const char * pchUserConfigDir)
{		
	//return false;
	auto ret = false;
	hid_init();
	hid_device *handle = hid_open(0x104d, 0x1974, NULL);
	if (handle)
	{
		ret = true;;
		hid_close(handle);
	}
	hid_exit();
	//if (ret)
	//{
	//	//monitor not detected if directmode active, so skipping mon detection
	//	//CDummyLog g_DuymmyLog;
	//	//HMDLog tLog(&g_DuymmyLog);	
	//	HMDData hmdData = { 0 };
	//	wcscpy_s(hmdData.Model, L"SNYD602");
	//	//hmdData.Logger = &tLog;
	//	//if (hmdData.Logger && pchUserConfigDir) hmdData.Logger->Log(pchUserConfigDir);
	//	//if (hmdData.Logger) hmdData.Logger->Log("Enumnerating monitors...\n");
	//	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&hmdData);
	//	ret &= hmdData.IsConnected;
	//}
	return ret;
}

EVRInitError CClientDriver::SetDisplayId(const char * pchDisplayId)
{
	logger_->Log(__FUNCTION__"\n");
	//EnableFakePack();		
	//MessageBox(NULL, L"Displayid set", wstr.c_str(), 0);	
	return vr::VRInitError_None;
}

HiddenAreaMesh_t CClientDriver::GetHiddenAreaMesh(EVREye eEye)
{
	logger_->Log(__FUNCTION__"\n");
	vr::HiddenAreaMesh_t hidden_area_mesh;
	hidden_area_mesh.pVertexData = nullptr;
	hidden_area_mesh.unTriangleCount = 0;
	return hidden_area_mesh;
}

uint32_t CClientDriver::GetMCImage(uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen)
{
	logger_->Log(__FUNCTION__"\n");
	return 0;
}
