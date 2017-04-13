#include "TrackedDevice.h"

CTrackedDevice::CTrackedDevice(std::string displayName, CServerDriver *pServer)
{
	//m_pLog = vr::VRDriverLog(); // pServer->m_pLog;
	InitDriverLog(vr::VRDriverLog());
	DriverLog(__FUNCTION__" %s", displayName.c_str());

	
	m_DisplayName = displayName;	
	m_pServer = pServer;
	m_pDriverHost = pServer->m_pDriverHost;

	m_pSettings = vr::VRSettings(); // m_pDriverHost ? m_pDriverHost->GetSettings(IVRSettings_Version) : nullptr;

	//m_KeyDown = false;
	//m_LastDown = GetTickCount();
	//m_Delay = 500;

	m_pProperties = vr::VRProperties();

	m_unObjectId = k_unTrackedDeviceIndexInvalid;
	m_ulPropertyContainer = 0;
	
	IconPathName = "icons";
	TrackingSystemName = "Custom HMD";
	ModelNumber = "0";
	SerialNumber = "0";
	RenderModelName = "0";			
	AllWirelessDongleDescriptions = "";
	ConnectedWirelessDongle = "";	
	Firmware_ProgrammingTarget = "";
	ManufacturerName = "COLTU";
	TrackingFirmwareVersion = "2.2.0";
	HardwareRevision = 2;		
	HardwareRevisionS = "2.0.0";
	Firmware_ManualUpdateURL = "https://github.com/sencercoltu/openvr-customhmd";	
	DriverVersion = "1.0.6";
	BlockServerShutdown = false;
	WillDriftInYaw = false;
	DeviceIsWireless = false;
	DeviceIsCharging = false;
	Firmware_UpdateAvailable = false;
	Firmware_ManualUpdate = true;
	CanUnifyCoordinateSystemWithHmd = false;
	ContainsProximitySensor = false;
	DeviceProvidesBatteryStatus = false;
	DeviceCanPowerOff = false;
	HasCamera = false;
	Firmware_ForceUpdateRequired = false;
	ViveSystemButtonFixRequired = false;
	DeviceBatteryPercentage = 1.0f;
	StatusDisplayTransform = HmdMatrix34_t();
	Quaternion::HmdMatrix_SetIdentity(&StatusDisplayTransform);
	DeviceClass = TrackedDeviceClass_Invalid;
	FirmwareVersion = 2;
	FPGAVersion = 0;
	VRCVersion = 0;
	RadioVersion = 0;
	DongleVersion = 0;	
	ParentDriver = 0;
}

void CTrackedDevice::SetDefaultProperties()
{
	ETrackedPropertyError error;
	m_ulPropertyContainer = m_pProperties->TrackedDeviceToPropertyContainer(m_unObjectId);
	
	error = SET_PROP(String, IconPathName, .c_str());
	error = SET_PROP(String, TrackingSystemName, .c_str());
	error = SET_PROP(String, ModelNumber, .c_str());
	error = SET_PROP(String, SerialNumber, .c_str());
	error = SET_PROP(String, RenderModelName, .c_str());
	error = SET_PROP(String, AllWirelessDongleDescriptions, .c_str());
	error = SET_PROP(String, ConnectedWirelessDongle, .c_str());
	error = SET_PROP(String, Firmware_ProgrammingTarget, .c_str());
	error = SET_PROP(String, ManufacturerName, .c_str());
	error = SET_PROP(String, TrackingFirmwareVersion, .c_str());
	error = m_pProperties->SetStringProperty(m_ulPropertyContainer, Prop_HardwareRevision_String, HardwareRevisionS.c_str());	
	error = SET_PROP(String, Firmware_ManualUpdateURL, .c_str());
	error = SET_PROP(String, DriverVersion, .c_str());

	error = SET_PROP(Bool, BlockServerShutdown,);
	error = SET_PROP(Bool, WillDriftInYaw,);
	error = SET_PROP(Bool, DeviceIsWireless,);
	error = SET_PROP(Bool, DeviceIsCharging,);
	error = SET_PROP(Bool, Firmware_UpdateAvailable,);
	error = SET_PROP(Bool, Firmware_ManualUpdate,);
	error = SET_PROP(Bool, CanUnifyCoordinateSystemWithHmd,);
	error = SET_PROP(Bool, ContainsProximitySensor,);
	error = SET_PROP(Bool, DeviceProvidesBatteryStatus,);
	error = SET_PROP(Bool, DeviceCanPowerOff,);
	error = SET_PROP(Bool, HasCamera,);
	error = SET_PROP(Bool, Firmware_ForceUpdateRequired,);
	error = SET_PROP(Bool, ViveSystemButtonFixRequired,);

	error = SET_PROP(Float, DeviceBatteryPercentage,);
	error = m_pProperties->SetProperty(m_ulPropertyContainer, Prop_StatusDisplayTransform_Matrix34, &StatusDisplayTransform, sizeof(StatusDisplayTransform), k_unHmdMatrix34PropertyTag);
	error = SET_PROP(Int32, DeviceClass,);
	
	error = SET_PROP(Uint64, HardwareRevision,);
	error = SET_PROP(Uint64, FirmwareVersion,);
	error = SET_PROP(Uint64, FPGAVersion,);
	error = SET_PROP(Uint64, VRCVersion,);
	error = SET_PROP(Uint64, RadioVersion,);
	error = SET_PROP(Uint64, DongleVersion,);
	error = SET_PROP(Uint64, ParentDriver,);
}

CTrackedDevice::~CTrackedDevice()
{
	//DriverLog(__FUNCTION__" %s", m_DisplayName.c_str());
	CleanupDriverLog();
	//m_pLog = nullptr;
}
//
//bool CTrackedDevice::GetBoolTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
//{
//	SET_ERROR(TrackedProp_UnknownProperty);
//
//	bool bValue = GetBoolProperty(prop, pError);
//
//	if (pError && *pError == TrackedProp_NotYetAvailable)
//	{
//		switch (prop)
//		{
//			case Prop_WillDriftInYaw_Bool:
//			case Prop_DeviceIsWireless_Bool:
//			case Prop_DeviceIsCharging_Bool:
//			case Prop_Firmware_UpdateAvailable_Bool:
//			case Prop_Firmware_ManualUpdate_Bool:
//			case Prop_BlockServerShutdown_Bool:
//			case Prop_CanUnifyCoordinateSystemWithHmd_Bool:
//			case Prop_ContainsProximitySensor_Bool:
//			case Prop_DeviceProvidesBatteryStatus_Bool:
//			case Prop_DeviceCanPowerOff_Bool:
//			case Prop_HasCamera_Bool:
//			case Prop_Firmware_ForceUpdateRequired_Bool:
//				SET_ERROR(TrackedProp_Success);
//				return false;
//			default:
//				SET_ERROR(TrackedProp_UnknownProperty);
//				break;
//		}
//	}
//	return bValue;
//}
//
//float CTrackedDevice::GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
//{
//	SET_ERROR(TrackedProp_UnknownProperty);
//	float fValue = GetFloatProperty(prop, pError);
//
//	if (pError && *pError == TrackedProp_NotYetAvailable)
//	{
//		switch (prop)
//		{
//			case Prop_DeviceBatteryPercentage_Float:
//				SET_ERROR(TrackedProp_Success);
//				return 1.0f;
//			default:
//				SET_ERROR(TrackedProp_UnknownProperty);
//				break;
//		}
//	}
//
//	return fValue;
//}
//
//int32_t CTrackedDevice::GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
//{
//	SET_ERROR(TrackedProp_UnknownProperty);
//	int32_t iValue = GetInt32Property(prop, pError);
//
//	if (pError && *pError == TrackedProp_NotYetAvailable)
//	{
//		switch (prop)
//		{
//			case Prop_DeviceClass_Int32:			
//				SET_ERROR(TrackedProp_Success);
//				return TrackedDeviceClass_Invalid;
//			default:
//				SET_ERROR(TrackedProp_UnknownProperty);
//				break;
//		}
//	}
//
//	return iValue;
//}
//
//uint64_t CTrackedDevice::GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
//{
//	SET_ERROR(TrackedProp_UnknownProperty);	
//	uint64_t iValue = GetUint64Property(prop, pError);
//
//	if (pError && *pError == TrackedProp_NotYetAvailable)
//	{
//		switch (prop) 
//		{
//			case Prop_HardwareRevision_Uint64:				
//			case Prop_FirmwareVersion_Uint64:				
//			case Prop_FPGAVersion_Uint64:				
//			case Prop_VRCVersion_Uint64:				
//			case Prop_RadioVersion_Uint64:				
//			case Prop_DongleVersion_Uint64:
//				SET_ERROR(TrackedProp_Success);
//				return 1;
//			default:
//				SET_ERROR(TrackedProp_UnknownProperty);
//				break;
//		}
//	}
//
//	return iValue;
//}
//
//HmdMatrix34_t CTrackedDevice::GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
//{
//	HmdMatrix34_t default_value;
//	Quaternion::HmdMatrix_SetIdentity(&default_value);
//
//	switch (prop) 
//	{
//	case Prop_StatusDisplayTransform_Matrix34:
//		SET_ERROR(TrackedProp_ValueNotProvidedByDevice);
//		return default_value;
//	}
//
//	SET_ERROR(TrackedProp_ValueNotProvidedByDevice);
//	return default_value;
//}
//
//uint32_t CTrackedDevice::GetStringTrackedDeviceProperty(ETrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, ETrackedPropertyError *pError)
//{	
//	SET_ERROR(TrackedProp_UnknownProperty);
//
//	std::string sValue = GetStringProperty(prop, pError);
//
//	if (pError && *pError != TrackedProp_NotYetAvailable)
//	{
//		switch (prop)
//		{
//			case Prop_TrackingSystemName_String:
//				sValue = Prop_TrackingSystemName;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_ModelNumber_String:
//				sValue = Prop_ModelNumber;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_SerialNumber_String:
//				sValue = Prop_SerialNumber;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_RenderModelName_String:
//				sValue = Prop_RenderModelName;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_ManufacturerName_String:
//				sValue = Prop_ManufacturerName;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_TrackingFirmwareVersion_String:
//				sValue = Prop_TrackingFirmwareVersion;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_HardwareRevision_String:
//				sValue = Prop_HardwareRevision;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_AllWirelessDongleDescriptions_String:
//				sValue = Prop_AllWirelessDongleDescriptions;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_ConnectedWirelessDongle_String:
//				sValue = Prop_ConnectedWirelessDongle;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_Firmware_ManualUpdateURL_String:
//				sValue = Prop_Firmware_ManualUpdateURL;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_Firmware_ProgrammingTarget_String:
//				sValue = Prop_Firmware_ProgrammingTarget;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			case Prop_DriverVersion_String:
//				sValue = Prop_DriverVersion;
//				SET_ERROR(TrackedProp_Success);
//				break;
//			default:
//				SET_ERROR(TrackedProp_UnknownProperty);
//				break;
//		}
//	}
//
//	if (pError && *pError == TrackedProp_Success)
//	{
//		if (sValue.size() + 1 > unBufferSize)
//		{
//			SET_ERROR(TrackedProp_BufferTooSmall);
//		}
//		else
//		{
//			strcpy_s(pchValue, unBufferSize, sValue.c_str());
//		}
//		return (uint32_t)sValue.size() + 1;
//	}
//	return 0;
//}

void CTrackedDevice::EnterStandby()
{
	DriverLog(__FUNCTION__" %s", m_DisplayName.c_str());
}

void CTrackedDevice::SendButtonUpdates(ButtonUpdate ButtonEvent, uint64_t ulMask)
{
	for (int i = 0; i < vr::k_EButton_Max; i++)
	{
		vr::EVRButtonId button = (vr::EVRButtonId)i;
		uint64_t bit = ButtonMaskFromId(button);
		if (bit & ulMask)
			(m_pDriverHost->*ButtonEvent)(m_unObjectId, button, 0.0);
	}
}

