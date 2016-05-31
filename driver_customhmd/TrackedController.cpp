#include <process.h>
#include "TrackedController.h"

CTrackedController::CTrackedController(ETrackedControllerRole role, std::string displayName, CServerDriver *pServer) : CTrackedDevice(displayName, pServer)
{
	_role = role;

	Prop_TrackingSystemName = "Nunchuck Controller";
	Prop_ModelNumber = "Nunchuck";
	Prop_SerialNumber = std::string(role == TrackedControllerRole_LeftHand ? "L" : "R").append("CTR-1244244");
	Prop_RenderModelName = "vr_controller_vive_1_5";
	Prop_ManufacturerName = "Wii";
	Prop_AllWirelessDongleDescriptions = std::string(role == TrackedControllerRole_LeftHand ? "L" : "R").append("CTR-None");
	Prop_ConnectedWirelessDongle = std::string(role == TrackedControllerRole_LeftHand ? "L" : "R").append("CTR-None");
	Prop_Firmware_ProgrammingTarget = std::string(role == TrackedControllerRole_LeftHand ? "L" : "R").append("CTR-Multi");

	ZeroMemory(&m_ControllerData, sizeof(m_ControllerData));
	switch (role)
	{
	case ETrackedControllerRole::TrackedControllerRole_LeftHand:
		m_ControllerData.hPoseLock = CreateMutex(NULL, FALSE, L"LeftPoseLock");
		m_ControllerData.Pose.vecPosition[0] = -0.2;
		m_ControllerData.Pose.vecPosition[1] = -0.2;
		m_ControllerData.Pose.vecPosition[2] = -0.5;
		break;
	case ETrackedControllerRole::TrackedControllerRole_RightHand:
		m_ControllerData.hPoseLock = CreateMutex(NULL, FALSE, L"RightPoseLock");
		m_ControllerData.Pose.vecPosition[0] = 0.2;
		m_ControllerData.Pose.vecPosition[1] = -0.2;
		m_ControllerData.Pose.vecPosition[2] = -0.5;
		break;
	}

	m_ControllerData.Pose.willDriftInYaw = false;
	m_ControllerData.Pose.shouldApplyHeadModel = false;
	m_ControllerData.Pose.deviceIsConnected = true;
	m_ControllerData.Pose.poseIsValid = true;
	m_ControllerData.Pose.result = ETrackingResult::TrackingResult_Running_OK;
	m_ControllerData.Pose.qRotation = Quaternion(1, 0, 0, 0);
	m_ControllerData.Pose.qWorldFromDriverRotation = Quaternion(1, 0, 0, 0);
	m_ControllerData.Pose.qDriverFromHeadRotation = Quaternion(1, 0, 0, 0);

	m_pServer->m_pDriverHost->TrackedDeviceAdded(Prop_SerialNumber.c_str());
}

CTrackedController::~CTrackedController()
{
	//Deactivate();	
	CloseHandle(m_ControllerData.hPoseLock);
}

EVRInitError CTrackedController::Activate(uint32_t unObjectId)
{
	m_unObjectId = unObjectId;
	return VRInitError_None;
}

void CTrackedController::Deactivate()
{
	m_unObjectId = k_unTrackedDeviceIndexInvalid;
}

void CTrackedController::PowerOff()
{
}

void * CTrackedController::GetComponent(const char * pchComponentNameAndVersion)
{
	if (!_stricmp(pchComponentNameAndVersion, IVRControllerComponent_Version))
	{
		return (IVRControllerComponent*)this;
	}

	// override this to add a component to a driver
	return nullptr;
}

void CTrackedController::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

DriverPose_t CTrackedController::GetPose()
{
	DriverPose_t pose;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_ControllerData.hPoseLock, INFINITE))
	{
		pose = m_ControllerData.Pose;
		ReleaseMutex(m_ControllerData.hPoseLock);
	}
	else
		return m_ControllerData.Pose;
	return pose;
}

bool CTrackedController::GetBoolProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	switch (prop)
	{
	case Prop_DeviceIsWireless_Bool:
		SET_ERROR(TrackedProp_Success);
		return false;
	case Prop_ContainsProximitySensor_Bool:
		SET_ERROR(TrackedProp_Success);
		return true;
	}
	SET_ERROR(TrackedProp_NotYetAvailable);
	return false;
}

float CTrackedController::GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	SET_ERROR(TrackedProp_NotYetAvailable);
	return 0.0f;
}

int32_t CTrackedController::GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	switch (prop)
	{
	case Prop_DeviceClass_Int32:
		SET_ERROR(TrackedProp_Success);
		return TrackedDeviceClass_Controller;
	case Prop_Axis0Type_Int32:
		SET_ERROR(TrackedProp_Success);
		return EVRControllerAxisType::k_eControllerAxis_TrackPad;
	case Prop_Axis1Type_Int32:
		SET_ERROR(TrackedProp_Success);
		return EVRControllerAxisType::k_eControllerAxis_Trigger;
	case Prop_Axis2Type_Int32:
	case Prop_Axis3Type_Int32:
	case Prop_Axis4Type_Int32:
		SET_ERROR(TrackedProp_Success);
		return EVRControllerAxisType::k_eControllerAxis_None;
	}
	SET_ERROR(TrackedProp_NotYetAvailable);
	return 0;
}

uint64_t CTrackedController::GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	switch (prop) {
	case Prop_SupportedButtons_Uint64:
		SET_ERROR(TrackedProp_Success);
		return
			ButtonMaskFromId(k_EButton_System) |
			ButtonMaskFromId(k_EButton_ApplicationMenu) |
			ButtonMaskFromId(k_EButton_Grip) |
			ButtonMaskFromId(k_EButton_Axis0) |
			ButtonMaskFromId(k_EButton_Axis1) |
			ButtonMaskFromId(k_EButton_Axis2) |
			ButtonMaskFromId(k_EButton_Axis3) |
			ButtonMaskFromId(k_EButton_Axis4);
			//k_EButton_DPad_Left = 3,
			//k_EButton_DPad_Up = 4,
			//k_EButton_DPad_Right = 5,
			//k_EButton_DPad_Down = 6,
			//k_EButton_A = 7,
	}

	SET_ERROR(TrackedProp_NotYetAvailable);
	return 0;
}

std::string CTrackedController::GetStringProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	switch (prop)
	{
	case Prop_AttachedDeviceId_String:
		SET_ERROR(TrackedProp_Success);
		return Prop_SerialNumber;
	}
	SET_ERROR(TrackedProp_ValueNotProvidedByDevice);
	return "";
}

VRControllerState_t CTrackedController::GetControllerState()
{
	m_ControllerData.State.unPacketNum++;
	return m_ControllerData.State;
}

bool CTrackedController::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
	return true;
}

void CTrackedController::RunFrame(DWORD currTick)
{
	if (m_KeyDown && currTick - m_LastDown > m_Delay)
	{
		m_KeyDown = false;
	}

	if (VKD(VK_LCONTROL))
	{
		if (!m_KeyDown)
		{
			if (VKD(VK_DECIMAL) && _role == TrackedControllerRole_RightHand)
			{
				m_pDriverHost->ProximitySensorState(m_unObjectId, true);
				m_KeyDown = true;
			}
			if (VKD(VK_DELETE) && _role == TrackedControllerRole_RightHand)
			{				
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_Grip, 0);
				Sleep(10);
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_Grip, 0);
				m_KeyDown = true;
			}
			if (VKD(VK_UP))
			{
				m_ControllerData.Euler.v[2] -= 0.01;
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_DPad_Up, 0);
				Sleep(10);
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_DPad_Up, 0);
				m_KeyDown = true;
			}
			if (VKD(VK_DOWN))
			{
				m_ControllerData.Euler.v[2] += 0.01;
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_DPad_Down, 0);
				Sleep(10);
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_DPad_Down, 0);
				m_KeyDown = true;
			}
			if (VKD(VK_LEFT))
			{
				m_ControllerData.Euler.v[0] += 0.01;
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_DPad_Left, 0);
				Sleep(10);
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_DPad_Left, 0);
				m_KeyDown = true;
			}
			if (VKD(VK_RIGHT))
			{
				m_ControllerData.Euler.v[0] -= 0.01;
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_DPad_Right, 0);
				Sleep(10);
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_DPad_Right, 0);
				m_KeyDown = true;
			}
			if (VKD(VK_DIVIDE) && _role == TrackedControllerRole_LeftHand)
			{
				//system button
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_System, 0);
				Sleep(1);
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_System, 0);
				m_KeyDown = true;
			}
			if (VKD(VK_MULTIPLY) && _role == TrackedControllerRole_RightHand)
			{
				//app button
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_ApplicationMenu, 0);
				Sleep(1);
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_ApplicationMenu, 0);
				m_KeyDown = true;
			}
			if (VKD(VK_RETURN) && _role == TrackedControllerRole_RightHand)
			{
				m_ControllerData.State.ulButtonPressed = ButtonMaskFromId(k_EButton_Axis1);
				m_ControllerData.State.rAxis[1].x = 1.0;
				m_pDriverHost->TrackedDeviceButtonPressed(m_unObjectId, k_EButton_SteamVR_Trigger, 0);
				Sleep(10);
				m_ControllerData.State.ulButtonPressed = 0;
				m_ControllerData.State.rAxis[1].x = 0.0;
				m_pDriverHost->TrackedDeviceButtonUnpressed(m_unObjectId, k_EButton_SteamVR_Trigger, 0);
				m_KeyDown = true;
				
			}

			if (m_KeyDown)
			{
				m_LastDown = currTick;
				m_Delay /= 2;
				if (m_Delay < 2)
					m_Delay = 2;

				if (WAIT_OBJECT_0 == WaitForSingleObject(m_ControllerData.hPoseLock, INFINITE))
				{
					m_ControllerData.Pose.qRotation = Quaternion::FromEuler(m_ControllerData.Euler).UnitQuaternion();
					m_ControllerData.PoseUpdated = true;
					ReleaseMutex(m_ControllerData.hPoseLock);
				}
			}
			else
			{
				m_Delay = 500;
			}
		}
	}
	else
	{
		m_KeyDown = false;
		m_Delay = 500;
	}

	DriverPose_t pose;
	pose.poseIsValid = false;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_ControllerData.hPoseLock, INFINITE))
	{
		if (m_ControllerData.PoseUpdated)
		{
			pose = m_ControllerData.Pose;
			m_ControllerData.PoseUpdated = false;
		}
		ReleaseMutex(m_ControllerData.hPoseLock);
	}
	if (pose.poseIsValid)
	{
		m_pDriverHost->TrackedDevicePoseUpdated(m_unObjectId, pose);		
	}
}

void CTrackedController::PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler)
{
	return;
	if (pData->Source != _role)
		return;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_ControllerData.hPoseLock, INFINITE))
	{
		m_ControllerData.LastState = *pData;
		auto euler = Quaternion(m_ControllerData.LastState.Rotation).ToEuler();
		euler.v[0] = euler.v[0] + pCenterEuler->v[0];
		euler.v[1] = euler.v[1] + pCenterEuler->v[1];
		euler.v[2] = euler.v[2] + pCenterEuler->v[2];
		m_ControllerData.Pose.qRotation = Quaternion::FromEuler(euler).UnitQuaternion();
		m_ControllerData.PoseUpdated = true;
		ReleaseMutex(m_ControllerData.hPoseLock);
	}
}