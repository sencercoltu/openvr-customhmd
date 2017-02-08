#ifndef TrackedDevice_H
#define TrackedDevice_H

#pragma once

#include "Common.h"
#include "ServerDriver.h"	
#include "Quaternion.h"

using namespace vr; 

//#define SET_ERROR(a) if (pError) *pError = a;
#define SET_PROP(t,p,e) m_pProperties->Set##t##Property(m_ulPropertyContainer, Prop_##p##_##t##, ##p####e##);

class CTrackedDevice :	
	public ITrackedDeviceServerDriver,
	public CDriverLog
{
	friend class CServerDriver;
public:
	CTrackedDevice(std::string displayName, CServerDriver *pServer);
	virtual ~CTrackedDevice();
	
	//std::string m_Id;
	virtual void RunFrame(DWORD currTick) {}	
	//EVRInitError Activate(uint32_t unObjectId) override;
	//void Deactivate() override;
	void EnterStandby() override;
	//void *GetComponent(const char *pchComponentNameAndVersion) override;
	//void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;
	//DriverPose_t GetPose() override;
//private:
//	uint32_t GetStringTrackedDeviceProperty(ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, ETrackedPropertyError * pError) override;
//	bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
//	float GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
//	int32_t GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
//	uint64_t GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
//	HmdMatrix34_t GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;

protected:	
	//virtual std::string GetStringProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) = 0;
	//virtual bool GetBoolProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) = 0;
	//virtual float GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) = 0; 
	//virtual int32_t GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) = 0;
	//virtual uint64_t GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) = 0;

	virtual void PacketReceived(USBPacket *pPacket, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos) = 0;
protected:	
	struct SendButtonData
	{
		EVRButtonId k_EButton;
		CTrackedDevice *pController;
		int Duration;
	};
	typedef void (vr::IVRServerDriverHost::*ButtonUpdate)(uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);

	void SendButtonUpdates(ButtonUpdate ButtonEvent, uint64_t ulMask);
	CVRPropertyHelpers *m_pProperties;
	virtual void SetDefaultProperties();
	PropertyContainerHandle_t m_ulPropertyContainer;
	IVRSettings *m_pSettings;
	//CDriverLog *m_pLog;
	IVRServerDriverHost *m_pDriverHost;
	CServerDriver *m_pServer;
	uint32_t m_unObjectId;

	std::string m_DisplayName;

	std::string TrackingSystemName;
	std::string ModelNumber;
	std::string SerialNumber;
	std::string RenderModelName;
	std::string ManufacturerName;
	std::string TrackingFirmwareVersion;
	std::string HardwareRevisionS;
	std::string AllWirelessDongleDescriptions;
	std::string ConnectedWirelessDongle;
	std::string Firmware_ManualUpdateURL;
	std::string Firmware_ProgrammingTarget;
	std::string DriverVersion;
	bool WillDriftInYaw;
	bool DeviceIsWireless;
	bool DeviceIsCharging;
	bool Firmware_UpdateAvailable;
	bool Firmware_ManualUpdate;
	bool BlockServerShutdown;
	bool CanUnifyCoordinateSystemWithHmd;
	bool ContainsProximitySensor;
	bool DeviceProvidesBatteryStatus;
	bool DeviceCanPowerOff;
	bool HasCamera;
	bool Firmware_ForceUpdateRequired;
	bool ViveSystemButtonFixRequired;
	float DeviceBatteryPercentage;
	HmdMatrix34_t StatusDisplayTransform;
	uint64_t HardwareRevision;
	uint64_t FirmwareVersion;
	uint64_t FPGAVersion;
	uint64_t VRCVersion;
	uint64_t RadioVersion;
	uint64_t DongleVersion;
	uint64_t ParentDriver;
	int32_t DeviceClass;
};

#endif //TrackedDevice_H