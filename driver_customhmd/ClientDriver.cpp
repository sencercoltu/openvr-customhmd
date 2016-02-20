#include "ClientDriver.h"

EVRInitError CClientDriver::Init(IDriverLog * pDriverLog, IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{
	//TRACE(__FUNCTIONW__);
	logger_ = pDriverLog;
	driverHost_ = pDriverHost;
	userDriverConfigDir_ = pchUserDriverConfigDir;
	driverInstallDir_ = pchDriverInstallDir;
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
