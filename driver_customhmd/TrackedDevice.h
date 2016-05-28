#ifndef TrackedDevice_H
#define TrackedDevice_H

#pragma once

#include "Common.h"
#include "ServerDriver.h"	
#include "Quaternion.h"

using namespace vr; 

#define SET_ERROR(a) if (pError) *pError = a;

class CTrackedDevice :
	public virtual ITrackedDeviceServerDriver
{
	friend class CServerDriver;
public:
	CTrackedDevice(std::string id, CServerDriver *pServer);
	~CTrackedDevice();

	//std::string m_Id;
	virtual void RunFrame(DWORD currTick) {}

private:
	uint32_t GetStringTrackedDeviceProperty(ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, ETrackedPropertyError * pError) override;
	bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
	float GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	int32_t GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	uint64_t GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	HmdMatrix34_t GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;

protected:	
	virtual std::string GetStringProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) = 0;
	virtual bool GetBoolProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) = 0;
	virtual float GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) = 0; 
	virtual int32_t GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) = 0;
	virtual uint64_t GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) = 0;

	virtual void PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler) = 0;
protected:
	IDriverLog *m_pLog;
	IServerDriverHost *m_pDriverHost;
	CServerDriver *m_pServer;
	uint32_t m_unObjectId;

	bool m_KeyDown;
	DWORD m_LastDown;
	DWORD m_Delay;


	std::string Prop_TrackingSystemName;
	std::string Prop_ModelNumber;
	std::string Prop_SerialNumber;
	std::string Prop_RenderModelName;
	std::string Prop_ManufacturerName;
	std::string Prop_TrackingFirmwareVersion;
	std::string Prop_HardwareRevision;
	std::string Prop_AllWirelessDongleDescriptions;
	std::string Prop_ConnectedWirelessDongle;
	std::string Prop_Firmware_ManualUpdateURL;
	std::string Prop_Firmware_ProgrammingTarget;
	std::string Prop_DriverVersion;
};

#endif //TrackedDevice_H