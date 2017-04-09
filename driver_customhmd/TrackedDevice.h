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

public:	//ITrackedDeviceServerDriver
	void EnterStandby() override;

protected:
	virtual void PacketReceived(USBPacket *pPacket, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos) = 0;
	virtual void RunFrame(DWORD currTick) {}

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

	std::string NamedIconPathDeviceOff;
	std::string NamedIconPathDeviceSearching;
	std::string NamedIconPathDeviceSearchingAlert;
	std::string NamedIconPathDeviceReady;
	std::string NamedIconPathDeviceReadyAlert;
	std::string NamedIconPathDeviceNotReady;
	std::string NamedIconPathDeviceStandby;
	std::string NamedIconPathDeviceAlertLow;

	std::string IconPathName;
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