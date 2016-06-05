#pragma once

#include "TrackedDevice.h"

class CTrackedController : 
	public CTrackedDevice,
	public IVRControllerComponent
{
private:
	ETrackedControllerRole m_Role;
	ControllerData m_ControllerData;
	struct SendButtonData
	{		
		EVRButtonId k_EButton;
		CTrackedController *pController;
		int Duration;		
	};
	void SendButton(EVRButtonId k_EButton);
public:
	CTrackedController(ETrackedControllerRole role, std::string displayName, CServerDriver *pServer);
	~CTrackedController();
	
	EVRInitError Activate(uint32_t unObjectId) override;
	void Deactivate() override;
	void PowerOff() override;
	void * GetComponent(const char * pchComponentNameAndVersion) override;
	void DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) override;
	DriverPose_t GetPose() override;
	VRControllerState_t GetControllerState() override;
	bool TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) override;

protected:
	std::string GetStringProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
	bool GetBoolProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
	float GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	int32_t GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	uint64_t GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	void PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos) override;
	void RunFrame(DWORD currTick) override;
};

