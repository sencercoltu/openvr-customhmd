#include <process.h>
#include "TrackedController.h"

CTrackedController::CTrackedController(ETrackedControllerRole role, std::string id, CServerDriver *pServer) : CTrackedDevice(id, pServer)
{
	_role = role;

	m_hThread = nullptr;
	m_IsRunning = false;

	ZeroMemory(&m_ControllerData, sizeof(m_ControllerData));
	m_ControllerData.hPoseLock = CreateMutex(NULL, FALSE, role == ETrackedControllerRole::TrackedControllerRole_LeftHand?  L"LeftPoseLock" : L"RightPoseLock");
	m_ControllerData.Pose.deviceIsConnected = true;
	m_ControllerData.Pose.poseIsValid = true;
	m_ControllerData.Pose.result = ETrackingResult::TrackingResult_Running_OK;

	pServer->driverHost_->TrackedDeviceAdded(m_Id.c_str());
}

CTrackedController::~CTrackedController()
{
	//Deactivate();	
	CloseHandle(m_ControllerData.hPoseLock);
}

unsigned int WINAPI CTrackedController::ProcessThread(void *p)
{
	if (!p) return -1;
	auto device = static_cast<CTrackedController *>(p);
	if (device)
		device->Run();
	_endthreadex(0);
	return 0;
}

void CTrackedController::Run()
{
	auto keydown = false;
	auto lastdown = GetTickCount();
	DWORD delay = 1000;


	m_ControllerData.Pose.poseIsValid = true;
	m_ControllerData.Pose.result = ETrackingResult::TrackingResult_Running_OK;
	m_ControllerData.Pose.willDriftInYaw = false;
	m_ControllerData.Pose.shouldApplyHeadModel = true;

	while (m_IsRunning)
	{
		if (keydown && GetTickCount() - lastdown > delay)
		{
			keydown = false;
		}

		if (_role == TrackedControllerRole_RightHand && VKD(VK_LCONTROL))
		{
			if (!keydown)
			{
				if (VKD(VK_UP))
				{
					m_ControllerData.Euler.v[0]++;
					keydown = true;
				}
				else if (VKD(VK_DOWN))
				{
					m_ControllerData.Euler.v[0]--;
					keydown = true;
				}
				else if (VKD(VK_LEFT))
				{
					m_ControllerData.Euler.v[1]++;
					keydown = true;
				}
				else if (VKD(VK_RIGHT))
				{
					m_ControllerData.Euler.v[1]--;
					keydown = true;
				}
				else if (VKD(VK_DIVIDE))
				{
					//system button
					m_pDriverHost->TrackedDeviceButtonPressed(0, k_EButton_System, 0);
					Sleep(1);
					m_pDriverHost->TrackedDeviceButtonUnpressed(0, k_EButton_System, 0);
					keydown = true;
				}
				else if (VKD(VK_MULTIPLY))
				{
					//app button
					m_pDriverHost->TrackedDeviceButtonPressed(0, k_EButton_ApplicationMenu, 0);
					Sleep(1);
					m_pDriverHost->TrackedDeviceButtonUnpressed(0, k_EButton_ApplicationMenu, 0);
					keydown = true;
				}
				else
				{
					delay = 1000;
				}

				if (keydown)
				{
					lastdown = GetTickCount();
					delay /= 2;
					if (delay < 2)
						delay = 2;

					if (WAIT_OBJECT_0 == WaitForSingleObject(m_ControllerData.hPoseLock, INFINITE))
					{
						m_ControllerData.Pose.qRotation = Quaternion::FromEuler(m_ControllerData.Euler);
						m_ControllerData.PoseUpdated = true;
						ReleaseMutex(m_ControllerData.hPoseLock);
					}
				}
			}
		}
		else
		{
			keydown = false;
			delay = 1000;
		}
		Sleep(100);
	}
}

EVRInitError CTrackedController::Activate(uint32_t unObjectId)
{	
	m_ControllerData.Pose.poseIsValid = true;
	m_ControllerData.Pose.result = TrackingResult_Running_OK;
	m_ControllerData.Pose.deviceIsConnected = true;

	m_ControllerData.Pose.qWorldFromDriverRotation = Quaternion();
	m_ControllerData.Pose.qDriverFromHeadRotation = Quaternion();
	
	


	m_unObjectId = unObjectId;
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ProcessThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hThread);
	}
	return VRInitError_None;
}

void CTrackedController::Deactivate()
{
	m_IsRunning = false;
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}
	m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
}

void CTrackedController::PowerOff()
{
}

void * CTrackedController::GetComponent(const char * pchComponentNameAndVersion)
{
	if (!_stricmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version))
	{
		return (vr::IVRControllerComponent*)this;
	}

	// override this to add a component to a driver
	return nullptr;
}

void CTrackedController::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

DriverPose_t CTrackedController::GetPose()
{
	vr::DriverPose_t pose;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_ControllerData.hPoseLock, INFINITE))
	{
		pose = m_ControllerData.Pose;		
		ReleaseMutex(m_ControllerData.hPoseLock);
	}
	else
		return m_ControllerData.Pose;
	return pose;
}

bool CTrackedController::GetBoolTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	if (pError)
		*pError = vr::TrackedProp_ValueNotProvidedByDevice;
	return false;
}

float CTrackedController::GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	return 0.0f;
}

int32_t CTrackedController::GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	//	TRACE(__FUNCTIONW__);
	const int32_t default_value = 0;

	switch (prop)
	{
	case vr::Prop_DeviceClass_Int32:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return vr::TrackedDeviceClass_Controller;
	case vr::Prop_Axis0Type_Int32:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return vr::EVRControllerAxisType::k_eControllerAxis_TrackPad;
	case vr::Prop_Axis1Type_Int32:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return vr::EVRControllerAxisType::k_eControllerAxis_Trigger;
	case vr::Prop_Axis2Type_Int32:
	case vr::Prop_Axis3Type_Int32:
	case vr::Prop_Axis4Type_Int32:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return vr::EVRControllerAxisType::k_eControllerAxis_None;
	}

	if (pError)
		*pError = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint64_t CTrackedController::GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	//m_pLog->Log(__FUNCTION__"\n");
	const uint64_t default_value = 0;
	
	switch (prop) {
	case vr::Prop_CurrentUniverseId_Uint64:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return 2;
	case vr::Prop_CameraFirmwareVersion_Uint64:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return 1244245;
	case vr::Prop_PreviousUniverseId_Uint64:
	case vr::Prop_HardwareRevision_Uint64:
	case vr::Prop_FirmwareVersion_Uint64:
	case vr::Prop_FPGAVersion_Uint64:
	case vr::Prop_VRCVersion_Uint64:
	case vr::Prop_RadioVersion_Uint64:
	case vr::Prop_DongleVersion_Uint64:
	case vr::Prop_DisplayFPGAVersion_Uint64:
	case vr::Prop_DisplayBootloaderVersion_Uint64:
	case vr::Prop_DisplayHardwareVersion_Uint64:
	case vr::Prop_AudioFirmwareVersion_Uint64:
	case vr::Prop_SupportedButtons_Uint64:
	case vr::Prop_DisplayFirmwareVersion_Uint64:
		if (pError)
			*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (pError)
		*pError = vr::TrackedProp_UnknownProperty;
	return default_value;
}

HmdMatrix34_t CTrackedController::GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	//m_pLog->Log(__FUNCTION__"\n");
	// Default value is identity matrix
	vr::HmdMatrix34_t default_value;	

	switch (prop) {
	case vr::Prop_StatusDisplayTransform_Matrix34:
		if (pError)
			*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (pError)
		*pError = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint32_t CTrackedController::GetStringTrackedDeviceProperty(ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, ETrackedPropertyError * pError)
{
	std::string sValue = GetStringTrackedDeviceProperty(prop, pError);
	if (*pError == vr::TrackedProp_Success)
	{
		if (sValue.size() + 1 > unBufferSize)
		{
			*pError = vr::TrackedProp_BufferTooSmall;
		}
		else
		{
			strcpy_s(pchValue, unBufferSize, sValue.c_str());
		}
		return (uint32_t)sValue.size() + 1;
	}
	return 0;
}

std::string CTrackedController::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError)
{
	*pError = vr::TrackedProp_ValueNotProvidedByDevice;
	std::string sRetVal;

	switch (prop)
	{
	case vr::Prop_AttachedDeviceId_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return m_Id;
	case vr::Prop_ManufacturerName_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return "CUSTOM CONTROLLER";
	case vr::Prop_ModelNumber_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return "Model-1";
	case vr::Prop_SerialNumber_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return m_Id;
	case vr::Prop_TrackingSystemName_String:
	case vr::Prop_HardwareRevision_String:
	case vr::Prop_TrackingFirmwareVersion_String:
	case vr::Prop_RenderModelName_String:
	case vr::Prop_AllWirelessDongleDescriptions_String:
	case vr::Prop_ConnectedWirelessDongle_String:
	case vr::Prop_Firmware_ManualUpdateURL_String:
	case vr::Prop_Firmware_ProgrammingTarget_String:
	case vr::Prop_DisplayMCImageLeft_String:
	case vr::Prop_DisplayMCImageRight_String:
	case vr::Prop_DisplayGCImage_String:
	case vr::Prop_CameraFirmwareDescription_String:
	case vr::Prop_ModeLabel_String:
	default:
		if (pError)
			*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return sRetVal;
	}
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

void CTrackedController::RunFrame()
{
	vr::DriverPose_t pose;
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
