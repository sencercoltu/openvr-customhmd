#ifndef TrackedDevice_H
#define TrackedDevice_H

#pragma once

#include "ServerDriver.h"	

using namespace vr;

class CTrackedDevice :
	public virtual ITrackedDeviceServerDriver
{
public:
	CTrackedDevice(std::string id, CServerDriver *pServer)
	{
		m_Id = id ;  
		m_pDriverHost = pServer->driverHost_;
		m_pLog = pServer->logger_;
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
	}
	std::string m_Id;
	virtual void RunFrame() {}
protected:
	virtual std::string GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) = 0;
	IDriverLog *m_pLog;
	IServerDriverHost *m_pDriverHost;
	uint32_t m_unObjectId;

};

#endif //TrackedDevice_H