#include <process.h>
#include "TrackedController.h"

CTrackedController::CTrackedController(ETrackedControllerRole role, std::string displayName, CServerDriver *pServer) : CTrackedDevice(displayName, pServer)
{
	_LOG(__FUNCTION__);
	m_Role = role;

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
		m_ControllerData.Pose.vecPosition[0] = 0.2;
		m_ControllerData.Pose.vecPosition[1] = -0.2;
		m_ControllerData.Pose.vecPosition[2] = -0.5;
		break;
	case ETrackedControllerRole::TrackedControllerRole_RightHand:
		m_ControllerData.hPoseLock = CreateMutex(NULL, FALSE, L"RightPoseLock");
		m_ControllerData.Pose.vecPosition[0] = -0.2;
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
	_LOG(__FUNCTION__);
	//Deactivate();	
	CloseHandle(m_ControllerData.hPoseLock);
}

EVRInitError CTrackedController::Activate(uint32_t unObjectId)
{
	_LOG(__FUNCTION__);
	m_unObjectId = unObjectId;
	return VRInitError_None;
}

void CTrackedController::Deactivate()
{
	_LOG(__FUNCTION__);
	m_unObjectId = k_unTrackedDeviceIndexInvalid;
}

void CTrackedController::PowerOff()
{
	_LOG(__FUNCTION__);		
	// m_pServer->RemoveTrackedDevice(this); no DriverHost->TrackedDeviceRemoved function present :((
}

void *CTrackedController::GetComponent(const char * pchComponentNameAndVersion)
{
	_LOG(__FUNCTION__);
	if (!_stricmp(pchComponentNameAndVersion, IVRControllerComponent_Version))
	{
		return (IVRControllerComponent*)this;
	}

	// override this to add a component to a driver
	return nullptr;
}

void CTrackedController::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
	_LOG(__FUNCTION__);
}

DriverPose_t CTrackedController::GetPose()
{
	_LOG(__FUNCTION__);
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
	case Prop_ControllerRoleHint_Int32:
		SET_ERROR(TrackedProp_Success); 
		return m_Role;		
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
			//ButtonMaskFromId(k_EButton_Axis0) |
			ButtonMaskFromId(k_EButton_Axis1) |
			//ButtonMaskFromId(k_EButton_Axis2) |
			//ButtonMaskFromId(k_EButton_Axis3) |
			//ButtonMaskFromId(k_EButton_Axis4) |
			ButtonMaskFromId(k_EButton_DPad_Left) |
			ButtonMaskFromId(k_EButton_DPad_Up) |
			ButtonMaskFromId(k_EButton_DPad_Right) |
			ButtonMaskFromId(k_EButton_DPad_Down) |
			ButtonMaskFromId(k_EButton_A
			);
	case Prop_HardwareRevision_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 0x81010500;
	case Prop_FirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1441055399;
	case Prop_FPGAVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 262;
	case Prop_DongleVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1461100729;
	case Prop_VRCVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1465809477;
	case Prop_RadioVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1466630404;
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
	case Prop_ConnectedWirelessDongle_String:
		SET_ERROR(TrackedProp_Success);
		return "8983038AF4";

	}
	SET_ERROR(TrackedProp_ValueNotProvidedByDevice);
	return "";
}

VRControllerState_t CTrackedController::GetControllerState()
{
	_LOG(__FUNCTION__);
	m_ControllerData.State.unPacketNum++;
	return m_ControllerData.State;
}

bool CTrackedController::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
	_LOG(__FUNCTION__" axId: %d, dur: %d", unAxisId, usPulseDurationMicroseconds);
	USBPacket *pPacket = new USBPacket;
	ZeroMemory(pPacket, sizeof(USBPacket));
	pPacket->Header.Type = m_Role | COMMAND_DATA;
	pPacket->Header.Crc8 = 0;
	pPacket->Header.Sequence = (uint16_t) GetTickCount(); //put timestamp as sequence to hopefully prevent duplicates on target
	pPacket->Command.Command = CMD_VIBRATE;
	pPacket->Command.Data.Vibration.Axis = unAxisId;
	pPacket->Command.Data.Vibration.Duration = usPulseDurationMicroseconds;
	SetPacketCrc(pPacket);
	m_pServer->SendUSBCommand(pPacket);
	return true;
}

void CTrackedController::RunFrame(DWORD currTick)
{
	if (m_KeyDown && currTick - m_LastDown > m_Delay)
	{
		m_KeyDown = false;
	}

	VRControllerState_t newState = m_ControllerData.State;
	newState.ulButtonPressed = newState.ulButtonTouched = 0;
		
	if (VKD(VK_LCONTROL))
	{	
		if (!m_KeyDown)
		{
			if (VKD(VK_DECIMAL) && m_Role == TrackedControllerRole_RightHand)
			{
				m_pDriverHost->ProximitySensorState(m_unObjectId, true);
				m_KeyDown = true;
			}
			if (VKD(VK_BACK) && m_Role == TrackedControllerRole_LeftHand)
			{
				newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_Grip);				
				m_KeyDown = true;
			}
			if (VKD(VK_UP))
			{
				if (VKD(VK_LSHIFT) && m_Role == TrackedControllerRole_LeftHand)				
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_DPad_Up);
				else
					m_ControllerData.Euler.v[2] -= 0.01;
				m_KeyDown = true;

			}
			if (VKD(VK_DOWN))
			{
				if (VKD(VK_LSHIFT) && m_Role == TrackedControllerRole_LeftHand)
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_DPad_Down);
				else
					m_ControllerData.Euler.v[2] += 0.01;
				m_KeyDown = true;
			}
			if (VKD(VK_LEFT))
			{
				if (VKD(VK_LSHIFT) && m_Role == TrackedControllerRole_LeftHand)
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_DPad_Left);
				else
					m_ControllerData.Euler.v[0] += 0.01;
				m_KeyDown = true;
			}
			if (VKD(VK_RIGHT))
			{
				if (VKD(VK_LSHIFT) && m_Role == TrackedControllerRole_LeftHand)
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_DPad_Right);
				else
					m_ControllerData.Euler.v[0] -= 0.01;
				m_KeyDown = true;

			}
			if (VKD(VK_DIVIDE) && m_Role == TrackedControllerRole_LeftHand)
			{
				//system button
				newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_System);
				m_KeyDown = true;
			}
			if (VKD(VK_MULTIPLY) && m_Role == TrackedControllerRole_LeftHand)
			{
				//app button
				newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_ApplicationMenu);
				m_KeyDown = true;
			}
			if (VKD(VK_RETURN) && m_Role == TrackedControllerRole_LeftHand)
			{
				if (VKD(VK_LSHIFT))
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_A);
				else
				{						
					m_Delay = 1000;
					//if (state > 0.1f)
						newState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_Axis1);
					//if (state > 0.8f)
						newState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_Axis1);

					newState.rAxis[1].x = 1.0f;

					//for (auto i = 0; i < 10; i++)
					//{
					//	m_ControllerData.State.rAxis[1].x = 0.1f * (i + 1);
					//	m_pDriverHost->TrackedDeviceAxisUpdated(m_unObjectId, 1, m_ControllerData.State.rAxis[1]);
					//}
					//for (auto i = 10; i > 0; i--)
					//{
					//	m_ControllerData.State.rAxis[1].x = 0.1f * i;
					//	m_pDriverHost->TrackedDeviceAxisUpdated(m_unObjectId, 1, m_ControllerData.State.rAxis[1]);
					//}
				}
				
					
				m_KeyDown = true;

			}

			if (m_KeyDown)
			{
				// All pressed buttons are touched
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
		newState.rAxis[1].x = 0;
		m_KeyDown = false;
		m_Delay = 500;
	}

	newState.unPacketNum = m_ControllerData.State.unPacketNum + 1;

	newState.ulButtonTouched |= newState.ulButtonPressed;

	uint64_t ulChangedTouched = newState.ulButtonTouched ^ m_ControllerData.State.ulButtonTouched;
	uint64_t ulChangedPressed = newState.ulButtonPressed ^ m_ControllerData.State.ulButtonPressed;

	SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonTouched, ulChangedTouched & newState.ulButtonTouched);
	SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonPressed, ulChangedPressed & newState.ulButtonPressed);
	SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUnpressed, ulChangedPressed & ~newState.ulButtonPressed);
	SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUntouched, ulChangedTouched & ~newState.ulButtonTouched);

	if (newState.rAxis[1].x != m_ControllerData.State.rAxis[1].x)
		m_pDriverHost->TrackedDeviceAxisUpdated(m_unObjectId, 1, newState.rAxis[1]);

	m_ControllerData.State = newState;


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

void CTrackedController::SendButtonUpdates(ButtonUpdate ButtonEvent, uint64_t ulMask) 
{
	if (!ulMask)
		return;

	for (int i = 0; i< vr::k_EButton_Max; i++)
	{
		vr::EVRButtonId button = (vr::EVRButtonId)i;

		uint64_t bit = ButtonMaskFromId(button);

		if (bit & ulMask)
		{
			(m_pDriverHost->*ButtonEvent)(m_unObjectId, button, 0.0);
		}
	}
}

void CTrackedController::PoseUpdate(USBPacket *pPacket, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos)
{
	if ((pPacket->Header.Type & 0x0F ) != m_Role)
		return;

	if (WAIT_OBJECT_0 == WaitForSingleObject(m_ControllerData.hPoseLock, INFINITE)) 
	{
		switch (pPacket->Header.Type & 0xF0)
		{
		case TRIGGER_DATA:
			{
				VRControllerState_t newState = m_ControllerData.State;
				newState.ulButtonPressed = newState.ulButtonTouched = 0;

				if ((pPacket->Trigger.Digital & BUTTON_0) == BUTTON_0)
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_System);
				if ((pPacket->Trigger.Digital & BUTTON_1) == BUTTON_1)
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_A);
				if ((pPacket->Trigger.Digital & BUTTON_2) == BUTTON_2)
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_ApplicationMenu);
				if ((pPacket->Trigger.Digital & BUTTON_3) == BUTTON_3)
					newState.ulButtonPressed |= vr::ButtonMaskFromId(k_EButton_Grip);

				if (pPacket->Trigger.Analog[0].x > 0)
				{
					newState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
					newState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
					newState.rAxis[1].x = pPacket->Trigger.Analog[0].x;
				}
				else
				{
					newState.ulButtonPressed &= ~vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
					newState.ulButtonTouched &= ~vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
					newState.rAxis[1].x = pPacket->Trigger.Analog[0].x;
				}

				newState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
				newState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
				newState.rAxis[0].x = pPacket->Trigger.Analog[1].x;
				newState.rAxis[0].y = pPacket->Trigger.Analog[1].y;


				newState.unPacketNum = m_ControllerData.State.unPacketNum + 1;
				newState.ulButtonTouched |= newState.ulButtonPressed;

				uint64_t ulChangedTouched = newState.ulButtonTouched ^ m_ControllerData.State.ulButtonTouched;
				uint64_t ulChangedPressed = newState.ulButtonPressed ^ m_ControllerData.State.ulButtonPressed;

				SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonTouched, ulChangedTouched & newState.ulButtonTouched);
				SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonPressed, ulChangedPressed & newState.ulButtonPressed);
				SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUnpressed, ulChangedPressed & ~newState.ulButtonPressed);
				SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUntouched, ulChangedTouched & ~newState.ulButtonTouched);

				if (newState.rAxis[1].x != m_ControllerData.State.rAxis[1].x)
					m_pDriverHost->TrackedDeviceAxisUpdated(m_unObjectId, 1, newState.rAxis[1]);
				if (newState.rAxis[0].x != m_ControllerData.State.rAxis[0].x || newState.rAxis[0].y != m_ControllerData.State.rAxis[0].y)
					m_pDriverHost->TrackedDeviceAxisUpdated(m_unObjectId, 0, newState.rAxis[0]);

				m_ControllerData.State = newState;

				m_ControllerData.LastState.Trigger = pPacket->Trigger;
			}
			break;
		case ROTATION_DATA:
		{
			m_ControllerData.LastState.Rotation = pPacket->Rotation;
			auto euler = Quaternion((float *)&m_ControllerData.LastState.Rotation).ToEuler();
			euler.v[0] = euler.v[0] + pCenterEuler->v[0];
			euler.v[1] = euler.v[1] + pCenterEuler->v[1];
			euler.v[2] = euler.v[2] + pCenterEuler->v[2];
			m_ControllerData.Pose.qRotation = Quaternion::FromEuler(euler).UnitQuaternion();
			memcpy(m_ControllerData.Pose.vecPosition, pRelativePos, sizeof(HmdVector3d_t));

			switch (m_Role)
			{
			case ETrackedControllerRole::TrackedControllerRole_RightHand:
				m_ControllerData.Pose.vecPosition[0] += 0.2;
				m_ControllerData.Pose.vecPosition[1] += -0.2;
				m_ControllerData.Pose.vecPosition[2] += -0.5;
				break;
			case ETrackedControllerRole::TrackedControllerRole_LeftHand:
				m_ControllerData.Pose.vecPosition[0] += -0.2;
				m_ControllerData.Pose.vecPosition[1] += -0.2;
				m_ControllerData.Pose.vecPosition[2] += -0.5;
				break;
			}

			m_ControllerData.PoseUpdated = true;
		}
		break;
		case POSITION_DATA:
		{
			m_ControllerData.LastState.Position = pPacket->Position;
		}
		break;
		case COMMAND_DATA:
			break;
		}

		ReleaseMutex(m_ControllerData.hPoseLock);
	}
}