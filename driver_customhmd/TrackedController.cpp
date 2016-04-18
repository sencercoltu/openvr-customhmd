#include "TrackedController.h"

CTrackedController::CTrackedController(ETrackedControllerRole role, std::string id, CServerDriver *pServer) : CTrackedDevice(id, pServer)
{	
	_role = role;
}

EVRInitError CTrackedController::Activate(uint32_t unObjectId)
{
	m_unObjectId = unObjectId;
	return VRInitError_None;
}

void CTrackedController::Deactivate()
{
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
	return DriverPose_t();
}

bool CTrackedController::GetBoolTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
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
	case vr::Prop_EdidVendorID_Int32:
	case vr::Prop_EdidProductID_Int32:
	case vr::Prop_DisplayMCType_Int32:
	case vr::Prop_DisplayGCType_Int32:
	case vr::Prop_Axis0Type_Int32:
	case vr::Prop_Axis1Type_Int32:
	case vr::Prop_Axis2Type_Int32:
	case vr::Prop_Axis3Type_Int32:
	case vr::Prop_Axis4Type_Int32:
		if (pError)
			*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (pError)
		*pError = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint64_t CTrackedController::GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	return uint64_t();
}

HmdMatrix34_t CTrackedController::GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	return HmdMatrix34_t();
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
		return "1244245";
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
	return VRControllerState_t();
}

bool CTrackedController::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
	return false;
}
