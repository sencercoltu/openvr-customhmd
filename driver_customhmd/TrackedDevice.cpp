#include "TrackedDevice.h"

CTrackedDevice::CTrackedDevice(std::string displayName, CServerDriver *pServer)
{
	m_pLog = pServer->m_pLog;

	_LOG(__FUNCTION__" %s", displayName.c_str());

	
	m_DisplayName = displayName;	
	m_pServer = pServer;
	m_pDriverHost = pServer->m_pDriverHost;

	m_pSettings = m_pDriverHost ? m_pDriverHost->GetSettings(IVRSettings_Version) : nullptr;

	m_KeyDown = false;
	m_LastDown = GetTickCount();
	m_Delay = 500;

	m_unObjectId = k_unTrackedDeviceIndexInvalid;

	Prop_TrackingFirmwareVersion = "2.2.0";
	Prop_HardwareRevision = "2.0.0";
	Prop_DriverVersion = "1.0.0";
	Prop_Firmware_ManualUpdateURL = "https://github.com/sencercoltu/openvr-customhmd";

}

CTrackedDevice::~CTrackedDevice()
{
	_LOG(__FUNCTION__" %s", m_DisplayName.c_str());
	m_pLog = nullptr;
}

bool CTrackedDevice::GetBoolTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	SET_ERROR(TrackedProp_UnknownProperty);

	bool bValue = GetBoolProperty(prop, pError);

	if (pError && *pError == TrackedProp_NotYetAvailable)
	{
		switch (prop)
		{
			case Prop_WillDriftInYaw_Bool:
			case Prop_DeviceIsWireless_Bool:
			case Prop_DeviceIsCharging_Bool:
			case Prop_Firmware_UpdateAvailable_Bool:
			case Prop_Firmware_ManualUpdate_Bool:
			case Prop_BlockServerShutdown_Bool:
			case Prop_CanUnifyCoordinateSystemWithHmd_Bool:
			case Prop_ContainsProximitySensor_Bool:
			case Prop_DeviceProvidesBatteryStatus_Bool:
			case Prop_DeviceCanPowerOff_Bool:
			case Prop_HasCamera_Bool:
			case Prop_Firmware_ForceUpdateRequired_Bool:
				SET_ERROR(TrackedProp_Success);
				return false;
			default:
				SET_ERROR(TrackedProp_UnknownProperty);
				break;
		}
	}
	return bValue;
}

float CTrackedDevice::GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	SET_ERROR(TrackedProp_UnknownProperty);
	float fValue = GetFloatProperty(prop, pError);

	if (pError && *pError == TrackedProp_NotYetAvailable)
	{
		switch (prop)
		{
			case Prop_DeviceBatteryPercentage_Float:
				SET_ERROR(TrackedProp_Success);
				return 1.0f;
			default:
				SET_ERROR(TrackedProp_UnknownProperty);
				break;
		}
	}

	return fValue;
}

int32_t CTrackedDevice::GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	SET_ERROR(TrackedProp_UnknownProperty);
	int32_t iValue = GetInt32Property(prop, pError);

	if (pError && *pError == TrackedProp_NotYetAvailable)
	{
		switch (prop)
		{
			case Prop_DeviceClass_Int32:			
				SET_ERROR(TrackedProp_Success);
				return TrackedDeviceClass_Invalid;
			default:
				SET_ERROR(TrackedProp_UnknownProperty);
				break;
		}
	}

	return iValue;
}

uint64_t CTrackedDevice::GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	SET_ERROR(TrackedProp_UnknownProperty);	
	uint64_t iValue = GetUint64Property(prop, pError);

	if (pError && *pError == TrackedProp_NotYetAvailable)
	{
		switch (prop) 
		{
			case Prop_HardwareRevision_Uint64:				
			case Prop_FirmwareVersion_Uint64:				
			case Prop_FPGAVersion_Uint64:				
			case Prop_VRCVersion_Uint64:				
			case Prop_RadioVersion_Uint64:				
			case Prop_DongleVersion_Uint64:
				SET_ERROR(TrackedProp_Success);
				return 1;
			default:
				SET_ERROR(TrackedProp_UnknownProperty);
				break;
		}
	}

	return iValue;
}

HmdMatrix34_t CTrackedDevice::GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	HmdMatrix34_t default_value;
	Quaternion::HmdMatrix_SetIdentity(&default_value);

	switch (prop) 
	{
	case Prop_StatusDisplayTransform_Matrix34:
		SET_ERROR(TrackedProp_ValueNotProvidedByDevice);
		return default_value;
	}

	SET_ERROR(TrackedProp_ValueNotProvidedByDevice);
	return default_value;
}

uint32_t CTrackedDevice::GetStringTrackedDeviceProperty(ETrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, ETrackedPropertyError *pError)
{	
	SET_ERROR(TrackedProp_UnknownProperty);

	std::string sValue = GetStringProperty(prop, pError);

	if (pError && *pError != TrackedProp_NotYetAvailable)
	{
		switch (prop)
		{
			case Prop_TrackingSystemName_String:
				sValue = Prop_TrackingSystemName;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_ModelNumber_String:
				sValue = Prop_ModelNumber;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_SerialNumber_String:
				sValue = Prop_SerialNumber;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_RenderModelName_String:
				sValue = Prop_RenderModelName;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_ManufacturerName_String:
				sValue = Prop_ManufacturerName;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_TrackingFirmwareVersion_String:
				sValue = Prop_TrackingFirmwareVersion;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_HardwareRevision_String:
				sValue = Prop_HardwareRevision;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_AllWirelessDongleDescriptions_String:
				sValue = Prop_AllWirelessDongleDescriptions;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_ConnectedWirelessDongle_String:
				sValue = Prop_ConnectedWirelessDongle;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_Firmware_ManualUpdateURL_String:
				sValue = Prop_Firmware_ManualUpdateURL;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_Firmware_ProgrammingTarget_String:
				sValue = Prop_Firmware_ProgrammingTarget;
				SET_ERROR(TrackedProp_Success);
				break;
			case Prop_DriverVersion_String:
				sValue = Prop_DriverVersion;
				SET_ERROR(TrackedProp_Success);
				break;
			default:
				SET_ERROR(TrackedProp_UnknownProperty);
				break;
		}
	}

	if (pError && *pError == TrackedProp_Success)
	{
		if (sValue.size() + 1 > unBufferSize)
		{
			SET_ERROR(TrackedProp_BufferTooSmall);
		}
		else
		{
			strcpy_s(pchValue, unBufferSize, sValue.c_str());
		}
		return (uint32_t)sValue.size() + 1;
	}
	return 0;
}

void CTrackedDevice::EnterStandby()
{
	_LOG(__FUNCTION__" %s", m_DisplayName.c_str());
}
