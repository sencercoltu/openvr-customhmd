#include "ServerDriver.h"


EVRInitError CServerDriver::Init(IDriverLog * pDriverLog, IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{
//	TRACE(__FUNCTIONW__);
	logger_ = pDriverLog;
	driverHost_ = pDriverHost;
	trackedDevices_.push_back(new CTrackedDevice("SONY HMZ-T2", this));	
	return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
//	TRACE(__FUNCTIONW__);
	trackedDevices_.clear();
}

uint32_t CServerDriver::GetTrackedDeviceCount()
{
//	TRACE(__FUNCTIONW__);
	return (uint32_t) trackedDevices_.size();
}

ITrackedDeviceServerDriver * CServerDriver::GetTrackedDeviceDriver(uint32_t unWhich, const char *pchInterfaceVersion)
{
	if (0 != _stricmp(pchInterfaceVersion, ITrackedDeviceServerDriver_Version))
		return nullptr;
	//	TRACE(__FUNCTIONW__);
	if (unWhich >= trackedDevices_.size())
		return nullptr;
	return trackedDevices_.at(unWhich);
}

ITrackedDeviceServerDriver * CServerDriver::FindTrackedDeviceDriver(const char * pchId, const char *pchInterfaceVersion)
{
	if (0 != _stricmp(pchInterfaceVersion, ITrackedDeviceServerDriver_Version))
		return nullptr;
//	TRACE(__FUNCTIONW__);
	for (auto iter = trackedDevices_.begin(); iter != trackedDevices_.end(); iter++)
	{
		if (0 == std::strcmp(pchId, (*iter)->m_Id.c_str()))
		{
			return *iter;
		}
	}
	return nullptr;
}

void CServerDriver::RunFrame()
{
	//TRACE(__FUNCTIONW__);
//	for (auto iter = trackedDevices_.begin(); iter != trackedDevices_.end(); iter++)
//		(*iter)->RunFrame();
}

bool CServerDriver::ShouldBlockStandbyMode()
{
//	TRACE(__FUNCTIONW__);
	return false;
}

void CServerDriver::EnterStandby()
{
//	TRACE(__FUNCTIONW__);
}

void CServerDriver::LeaveStandby()
{
//	TRACE(__FUNCTIONW__);
}
