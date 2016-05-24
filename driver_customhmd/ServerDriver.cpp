#include "ServerDriver.h"
#include "TrackedHMD.h"
#include "TrackedController.h"


EVRInitError CServerDriver::Init(IDriverLog * pDriverLog, IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{	
	logger_ = pDriverLog;

	logger_->Log(__FUNCTION__"\n");

	driverHost_ = pDriverHost;

	trackedDevices_.push_back(new CTrackedHMD("SONY HMZ-T2", this));	
	trackedDevices_.push_back(new CTrackedController(TrackedControllerRole_LeftHand, "LEFT CONTROLLER", this));
	trackedDevices_.push_back(new CTrackedController(TrackedControllerRole_RightHand, "RIGHT CONTROLLER", this));	
	return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
	logger_->Log(__FUNCTION__"\n");
	for (auto iter = trackedDevices_.begin(); iter != trackedDevices_.end(); iter++)
		delete (*iter);
	trackedDevices_.clear();
	driverHost_ = nullptr;
	logger_ = nullptr;
}

uint32_t CServerDriver::GetTrackedDeviceCount()
{
	logger_->Log(__FUNCTION__"\n");
	return (uint32_t) trackedDevices_.size();
}

ITrackedDeviceServerDriver * CServerDriver::GetTrackedDeviceDriver(uint32_t unWhich, const char *pchInterfaceVersion)
{
	logger_->Log(__FUNCTION__"\n");
	if (0 != _stricmp(pchInterfaceVersion, ITrackedDeviceServerDriver_Version))
		return nullptr;	
	if (unWhich >= trackedDevices_.size())
		return nullptr;
	return trackedDevices_.at(unWhich);
}

ITrackedDeviceServerDriver * CServerDriver::FindTrackedDeviceDriver(const char * pchId, const char *pchInterfaceVersion)
{
	logger_->Log(__FUNCTION__"\n");
	if (0 != _stricmp(pchInterfaceVersion, ITrackedDeviceServerDriver_Version))
		return nullptr;
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
	for (auto iter = trackedDevices_.begin(); iter != trackedDevices_.end(); iter++)
		(*iter)->RunFrame();
}

bool CServerDriver::ShouldBlockStandbyMode()
{
	logger_->Log(__FUNCTION__"\n");
	return false;
}

void CServerDriver::EnterStandby()
{
	logger_->Log(__FUNCTION__"\n");
}

void CServerDriver::LeaveStandby()
{
	logger_->Log(__FUNCTION__"\n");
}
