#include <process.h>
#include "TrackedController.h"

CTrackedController::CTrackedController(ETrackedControllerRole role, std::string id, CServerDriver *pServer) : CTrackedDevice(id, pServer)
{
	_role = role;

	Prop_TrackingSystemName = "Nunchuck Controller";
	Prop_ModelNumber = "Nunchuck";
	Prop_SerialNumber = std::string(role == TrackedControllerRole_LeftHand? "L":"R").append("CTR-1244244");
	Prop_RenderModelName = "CTR-NoName";
	Prop_ManufacturerName = "Wii";
	Prop_AllWirelessDongleDescriptions = "CTR-None";
	Prop_ConnectedWirelessDongle = "CTR-None";
	Prop_Firmware_ProgrammingTarget = "CTR-Multi";
	
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
	m_ControllerData.Pose.deviceIsConnected = false;
	m_ControllerData.Pose.poseIsValid = false;
	m_ControllerData.Pose.result = ETrackingResult::TrackingResult_Uninitialized;
	m_ControllerData.Pose.qRotation = Quaternion();
	m_ControllerData.Pose.qWorldFromDriverRotation = Quaternion();
	m_ControllerData.Pose.qDriverFromHeadRotation = Quaternion();

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
	m_ControllerData.Pose.poseIsValid = true;
	m_ControllerData.Pose.result = TrackingResult_Running_OK;
	m_ControllerData.Pose.deviceIsConnected = true;
	m_pDriverHost->TrackedDevicePropertiesChanged(m_unObjectId);
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
	if (!_stricmp(pchComponentNameAndVersion, IVRDisplayComponent_Version))
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
				ButtonMaskFromId(k_EButton_Grip);
				//k_EButton_DPad_Left = 3,
				//k_EButton_DPad_Up = 4,
				//k_EButton_DPad_Right = 5,
				//k_EButton_DPad_Down = 6,
				//k_EButton_A = 7,
				//k_EButton_Axis0 = 32,
				//k_EButton_Axis1 = 33,
				//k_EButton_Axis2 = 34,
				//k_EButton_Axis3 = 35,
				//k_EButton_Axis4 = 36,
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
			if (VKD(VK_UP))
			{
				m_ControllerData.Euler.v[2] -= 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_DOWN))
			{
				m_ControllerData.Euler.v[2] += 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_LEFT))
			{
				m_ControllerData.Euler.v[0] += 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_RIGHT))
			{
				m_ControllerData.Euler.v[0] -= 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_DIVIDE) && _role == TrackedControllerRole_LeftHand)
			{
				//system button
				m_pDriverHost->TrackedDeviceButtonPressed(0, k_EButton_System, 0);
				Sleep(1);
				m_pDriverHost->TrackedDeviceButtonUnpressed(0, k_EButton_System, 0);
				m_KeyDown = true;
			}
			else if (VKD(VK_MULTIPLY) && _role == TrackedControllerRole_RightHand)
			{
				//app button
				m_pDriverHost->TrackedDeviceButtonPressed(0, k_EButton_ApplicationMenu, 0);
				Sleep(1);
				m_pDriverHost->TrackedDeviceButtonUnpressed(0, k_EButton_ApplicationMenu, 0);
				m_KeyDown = true;
			}
			else if (VKD(VK_RETURN))
			{
				m_KeyDown = true;
			}
			else
			{
				m_Delay = 500;
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
		m_pDriverHost->TrackedDevicePoseUpdated(m_unObjectId, pose);
}

void CTrackedController::PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler)
{
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