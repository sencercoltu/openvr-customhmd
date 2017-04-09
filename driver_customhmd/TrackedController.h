#pragma once

#include "TrackedDevice.h"



class CTrackedController :
	public IVRControllerComponent,
	public CTrackedDevice
{
private:
	ETrackedControllerRole m_Role;
	ControllerData m_ControllerData;
public:
	CTrackedController(ETrackedControllerRole role, std::string displayName, CServerDriver *pServer);
	~CTrackedController();

public: //IVRControllerComponent
	VRControllerState_t GetControllerState() override;
	bool TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) override;

public:
	EVRInitError Activate(uint32_t unObjectId) override;
	void Deactivate() override;
	//void PowerOff() override;
	void * GetComponent(const char * pchComponentNameAndVersion) override;
	void DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) override;
	DriverPose_t GetPose() override;

protected:
	void PacketReceived(USBPacket *pPacket, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos) override;
	void RunFrame(DWORD currTick) override;

protected:
	void SetDefaultProperties() override;
	std::string AttachedDeviceId;
	uint64_t SupportedButtons;
	int32_t Axis0Type;
	int32_t Axis1Type;
	int32_t Axis2Type;
	int32_t Axis3Type;
	int32_t Axis4Type;
	int32_t ControllerRoleHint;
};

