#include <process.h>
//#include "Serial.h"
#include "TrackedHMD.h"


CTrackedHMD::CTrackedHMD(std::string id, CServerDriver *pServer) : CTrackedDevice(id, pServer)
{

	Prop_TrackingSystemName = "Sony HMZ-T2 HMD";
	Prop_ModelNumber = "HMZ-T2";
	Prop_SerialNumber = "HMD-1244244";
	Prop_RenderModelName = "HMD-NoName";
	Prop_ManufacturerName = "Sony";
	Prop_AllWirelessDongleDescriptions = "HMD-None";
	Prop_ConnectedWirelessDongle = "HMD-None";
	Prop_Firmware_ProgrammingTarget = "HMD-Multi";

	m_pSettings = m_pDriverHost ? m_pDriverHost->GetSettings(IVRSettings_Version) : nullptr;
		
	ZeroMemory(&m_HMDData, sizeof(m_HMDData));
	m_HMDData.IPDValue = 0.05f;
	m_HMDData.PosX = 0;
	m_HMDData.PosY = 0;
	m_HMDData.ScreenWidth = 1280;
	m_HMDData.ScreenHeight = 1470;
	m_HMDData.AspectRatio = ((float)(m_HMDData.ScreenHeight - 30) / 2.0f) / (float)m_HMDData.ScreenWidth;
	m_HMDData.Frequency = 120;
	m_HMDData.IsConnected = true;
	m_HMDData.FakePackDetected = true;
	m_HMDData.SuperSample = 1.0f;
	m_HMDData.PoseUpdated = false;
	m_HMDData.hPoseLock = CreateMutex(NULL, FALSE, L"PoseLock");

	//wcscpy_s(m_HMDData.Port, L"\\\\.\\COM3");
	wcscpy_s(m_HMDData.Model, L"SNYD602");

	m_HMDData.Pose = {};
	m_HMDData.Pose.willDriftInYaw = false;
	m_HMDData.Pose.shouldApplyHeadModel = false;
	m_HMDData.Pose.deviceIsConnected = false;
	m_HMDData.Pose.poseIsValid = false;
	m_HMDData.Pose.result = ETrackingResult::TrackingResult_Uninitialized;
	m_HMDData.Pose.qRotation = Quaternion();
	m_HMDData.Pose.qWorldFromDriverRotation = Quaternion();
	m_HMDData.Pose.qDriverFromHeadRotation = Quaternion();

	HMDLog *pLog = new HMDLog(m_pLog);

	if (m_pSettings)
	{
		char value[128];

		m_HMDData.DirectMode = m_pSettings->GetBool("steamvr", "directMode", false);

		value[0] = 0;
		m_pSettings->GetString("driver_customhmd", "monitor", value, sizeof(value), "SNYD602");
		if (value[0])
		{
			std::string basic_string(value);
			std::wstring wchar_value(basic_string.begin(), basic_string.end());
			wcscpy_s(m_HMDData.Model, wchar_value.c_str());

			pLog->Log(m_HMDData.Model);
			pLog->Log(" - HMD: MODEL OK\n");
		}

		/*value[0] = 0;  usb kullan
		pSettings->GetString("driver_customhmd", "port", value, sizeof(value), "\\\\.\\COM3");
		if (value[0])
		{
			std::string basic_string(value);
			std::wstring wchar_value(basic_string.begin(), basic_string.end());
			wcscpy_s(m_HMDData.Port, wchar_value.c_str());
			pLog->Log(m_HMDData.Port);
			pLog->Log(" - HMD: PORT OK\n");
		}*/

		m_HMDData.SuperSample = m_pSettings->GetFloat("driver_customhmd", "supersample", 1.0f);
	}

	m_HMDData.Logger = pLog;
	pLog->Log("HMD: Enumerating monitors...\n");
	if (!m_HMDData.DirectMode)
	{
		EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&m_HMDData);
		/*if (!m_HMDData.IsConnected)
		{
			m_HMDData.PosX = 0;
			m_HMDData.PosY = 0;
			m_HMDData.ScreenWidth = 1280/2;
			m_HMDData.ScreenHeight = 720/2;
			m_HMDData.Frequency = 60;
			m_HMDData.AspectRatio = (float)m_HMDData.ScreenWidth / (float)m_HMDData.ScreenHeight;
			m_HMDData.FakePackDetected = false;
		}*/
	}

	pLog->Log("HMD: Done.\n");

	m_pDriverHost->TrackedDeviceAdded(Prop_SerialNumber.c_str());
}

CTrackedHMD::~CTrackedHMD()
{
	//Deactivate();
	delete m_HMDData.Logger;
	CloseHandle(m_HMDData.hPoseLock);
}

EVRInitError CTrackedHMD::Activate(uint32_t unObjectId)
{
	m_pLog->Log(__FUNCTION__"\n");

	m_unObjectId = unObjectId;
	m_HMDData.Pose.poseIsValid = true;
	m_HMDData.Pose.result = TrackingResult_Running_OK;
	m_HMDData.Pose.deviceIsConnected = true;
	m_HMDData.IPDValue = m_pSettings->GetFloat("driver_customhmd", "IPD", 0.05f);
	m_pDriverHost->TrackedDevicePropertiesChanged(m_unObjectId);
	return VRInitError_None;
}

void CTrackedHMD::Deactivate()
{
	m_pLog->Log(__FUNCTION__"\n");
	m_unObjectId = k_unTrackedDeviceIndexInvalid;
	//	TRACE(__FUNCTIONW__);
}

void *CTrackedHMD::GetComponent(const char *pchComponentNameAndVersion)
{
	m_pLog->Log(__FUNCTION__"\n");
	if (!_stricmp(pchComponentNameAndVersion, IVRDisplayComponent_Version))
	{
		return (IVRDisplayComponent*)this;
	}

	// override this to add a component to a driver
	return nullptr;
}

void CTrackedHMD::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
	m_pLog->Log(__FUNCTION__"\n");
	*pnX = m_HMDData.PosX;
	*pnY = m_HMDData.PosY;
	*pnWidth = m_HMDData.ScreenWidth;
	*pnHeight = m_HMDData.ScreenHeight;
}

bool CTrackedHMD::IsDisplayOnDesktop()
{
	//m_HMDData.Logger->Log(__FUNCTION__": %s\n", m_HMDData.DirectMode ? "false" : "true");
	return !m_HMDData.DirectMode;
}

bool CTrackedHMD::IsDisplayRealDisplay()
{
	//m_pLog->Log(__FUNCTION__": true\n");
	return true;
}

void CTrackedHMD::GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight)
{
	m_pLog->Log(__FUNCTION__"\n");
	if (m_HMDData.FakePackDetected)
	{
		*pnWidth = m_HMDData.ScreenWidth;
		*pnHeight = (m_HMDData.ScreenHeight - 30) / 2;
	}
	else
	{
		*pnWidth = m_HMDData.ScreenWidth;
		*pnHeight = m_HMDData.ScreenHeight;
	}
	*pnWidth = uint32_t(*pnWidth * m_HMDData.SuperSample);
	*pnHeight = uint32_t(*pnHeight * m_HMDData.SuperSample);
}

void CTrackedHMD::GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
	m_pLog->Log(__FUNCTION__"\n");
	if (m_HMDData.FakePackDetected)
	{
		uint32_t h = (m_HMDData.ScreenHeight - 30) / 2;
		switch (eEye)
		{
		case EVREye::Eye_Left:
			*pnX = 0;
			*pnY = h + 30;
			*pnWidth = m_HMDData.ScreenWidth;
			*pnHeight = h;
			break;
		case EVREye::Eye_Right:
			*pnX = 0;
			*pnY = 0;
			*pnWidth = m_HMDData.ScreenWidth;
			*pnHeight = h;
			break;
		}
	}
	else
	{
		switch (eEye)
		{
		case EVREye::Eye_Left:
			*pnX = 0;
			*pnY = 0;
			*pnWidth = m_HMDData.ScreenWidth / 2;
			*pnHeight = m_HMDData.ScreenHeight;
			break;
		case EVREye::Eye_Right:
			*pnX = 0 + (m_HMDData.ScreenWidth / 2);
			*pnY = 0;
			*pnWidth = m_HMDData.ScreenWidth / 2;
			*pnHeight = m_HMDData.ScreenHeight;
			break;
		}
	}
}

void CTrackedHMD::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom)
{
	m_pLog->Log(__FUNCTION__"\n");
	if (m_HMDData.FakePackDetected)
	{
		switch (eEye)
		{
		case EVREye::Eye_Left:
			*pfLeft = -1.0f;
			*pfRight = 1.0f;
			*pfTop = -1.0f * m_HMDData.AspectRatio;
			*pfBottom = 1.0f * m_HMDData.AspectRatio;
			break;
		case EVREye::Eye_Right:
			*pfLeft = -1.0f;
			*pfRight = 1.0f;
			*pfTop = -1.0f * m_HMDData.AspectRatio;
			*pfBottom = 1.0f * m_HMDData.AspectRatio;
			break;
		}
	}
	else
	{
		switch (eEye)
		{
		case EVREye::Eye_Left:
			*pfLeft = -1.0f;
			*pfRight = 1.0f;
			*pfTop = -1.0f / m_HMDData.AspectRatio;
			*pfBottom = 1.0f / m_HMDData.AspectRatio;
			break;
		case EVREye::Eye_Right:
			*pfLeft = -1.0f;
			*pfRight = 1.0f;
			*pfTop = -1.0f / m_HMDData.AspectRatio;
			*pfBottom = 1.0f / m_HMDData.AspectRatio;
			break;
		}
	}
}

DistortionCoordinates_t CTrackedHMD::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	DistortionCoordinates_t coords = {};
	coords.rfRed[0] = fU;
	coords.rfRed[1] = fV;
	coords.rfBlue[0] = fU;
	coords.rfBlue[1] = fV;
	coords.rfGreen[0] = fU;
	coords.rfGreen[1] = fV;
	return coords;
}

DriverPose_t CTrackedHMD::GetPose()
{
	DriverPose_t pose;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
	{
		pose = m_HMDData.Pose;
		ReleaseMutex(m_HMDData.hPoseLock);
	}
	else
		return m_HMDData.Pose;
	return pose;
}

bool CTrackedHMD::GetBoolProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	switch (prop)
	{
		case Prop_IsOnDesktop_Bool:
			SET_ERROR(TrackedProp_Success);
			return !m_HMDData.DirectMode;
		case Prop_HasCamera_Bool:
			SET_ERROR(TrackedProp_Success);
			return true;
	}
	SET_ERROR(TrackedProp_NotYetAvailable);
	return false;
}

float CTrackedHMD::GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	switch (prop)
	{
		case Prop_UserHeadToEyeDepthMeters_Float:
		case Prop_SecondsFromVsyncToPhotons_Float:
			SET_ERROR(TrackedProp_Success);
			return 0.0f;
		case Prop_DisplayFrequency_Float:
			SET_ERROR(TrackedProp_Success);
			return m_HMDData.Frequency;
		case Prop_UserIpdMeters_Float:
			SET_ERROR(TrackedProp_Success);
			return m_HMDData.IPDValue;
		case Prop_DisplayMCOffset_Float:
		case Prop_DisplayMCScale_Float:
		case Prop_DisplayGCBlackClamp_Float:
		case Prop_DisplayGCOffset_Float:
		case Prop_DisplayGCScale_Float:
		case Prop_DisplayGCPrescale_Float:
		case Prop_LensCenterLeftU_Float:
		case Prop_LensCenterLeftV_Float:
		case Prop_LensCenterRightU_Float:
		case Prop_LensCenterRightV_Float:
		case Prop_ScreenshotHorizontalFieldOfViewDegrees_Float:
		case Prop_ScreenshotVerticalFieldOfViewDegrees_Float:
			SET_ERROR(TrackedProp_Success);
			return 0.0f;
	}
	SET_ERROR(TrackedProp_NotYetAvailable);
	return 0.0f;
}

int32_t CTrackedHMD::GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	switch (prop)
	{
		case Prop_DeviceClass_Int32:
			SET_ERROR(TrackedProp_Success);
			return TrackedDeviceClass_HMD;
		case Prop_EdidVendorID_Int32:
			SET_ERROR(TrackedProp_Success);
			return 0xD94D;
		case Prop_EdidProductID_Int32:		
			SET_ERROR(TrackedProp_Success);
			return 0xD602;
	}
	SET_ERROR(TrackedProp_NotYetAvailable);
	return 0;
}

uint64_t CTrackedHMD::GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	switch (prop) {
	case Prop_CurrentUniverseId_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 2;
	case Prop_CameraFirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1244245;
	case Prop_PreviousUniverseId_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 0;
	case Prop_DisplayFPGAVersion_Uint64:
	case Prop_DisplayBootloaderVersion_Uint64:
	case Prop_DisplayHardwareVersion_Uint64:
	case Prop_DisplayFirmwareVersion_Uint64:
	case Prop_AudioFirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1234;
	}



	SET_ERROR(TrackedProp_NotYetAvailable);
	return 0;
}

std::string CTrackedHMD::GetStringProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{	
	switch (prop)
	{
		case Prop_CameraFirmwareDescription_String:
			SET_ERROR(TrackedProp_Success);
			return "Sony HMZ-T2 Camera";
		case Prop_DisplayMCImageLeft_String:
		case Prop_DisplayMCImageRight_String:
		case Prop_DisplayGCImage_String:
			SET_ERROR(TrackedProp_ValueNotProvidedByDevice);		
			return "";			
	}
	SET_ERROR(TrackedProp_ValueNotProvidedByDevice);
	return "";
}

void CTrackedHMD::CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, void *(*pSharedTextureHandles)[3])
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::DestroySwapTextureSet(void *pSharedTextureHandle)
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::DestroyAllSwapTextureSets(uint32_t unPid)
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::GetNextSwapTextureSetIndex(void *pSharedTextureHandles[2], uint32_t(*pIndices)[2])
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::SubmitLayer(void *pSharedTextureHandles[2], const VRTextureBounds_t(&bounds)[2], const HmdMatrix34_t *pPose)
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::Present(void *hSyncTexture)
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::PowerOff()
{
	m_pLog->Log(__FUNCTION__"\n");
}

void CTrackedHMD::RunFrame(DWORD currTick)
{	
	const float step = 0.0001f;

	if (m_KeyDown && currTick - m_LastDown > m_Delay)
	{
		m_KeyDown = false;
	}

	if (VKD(VK_LCONTROL))
	{
		if (!m_KeyDown)
		{
			if (VKD(VK_ADD))
			{
				m_HMDData.IPDValue += step;
				m_pDriverHost->PhysicalIpdSet(0, m_HMDData.IPDValue);
				m_pSettings->SetFloat("driver_customhmd", "IPD", m_HMDData.IPDValue);
				m_pSettings->Sync(true);
				m_KeyDown = true;
			}
			else if (VKD(VK_SUBTRACT))
			{
				m_HMDData.IPDValue -= step;
				m_pDriverHost->PhysicalIpdSet(0, m_HMDData.IPDValue);
				m_pSettings->SetFloat("driver_customhmd", "IPD", m_HMDData.IPDValue);
				m_pSettings->Sync(true);
				m_KeyDown = true;
			}
			else if (VKD(VK_HOME))
			{
				auto euler = Quaternion(m_HMDData.LastState.Rotation).ToEuler();
				euler.v[0] *= -1;
				euler.v[1] *= -1;
				euler.v[2] *= -1;
				m_pServer->AlignHMD(&euler);

				////zero offset gyro
				//if (pHandle)
				//{
				//	buf[0] = 0;
				//	buf[1] = 1;
				//	hid_write(pHandle, buf, sizeof(buf));
				//}

				m_KeyDown = true;
			}
			else if (VKD(VK_END))
			{
				auto euler = HmdVector3d_t();
				m_pServer->AlignHMD(&euler);
				////reset gyro offset 
				//if (pHandle)
				//{
				//	buf[0] = 0;
				//	buf[1] = 1;
				//	hid_write(pHandle, buf, sizeof(buf));
				//}

				m_KeyDown = true;
			}
			else if (VKD(VK_NUMPAD6))
			{
				m_HMDData.Pose.vecPosition[0] += 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_NUMPAD4))
			{
				m_HMDData.Pose.vecPosition[0] -= 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_NUMPAD8))
			{
				m_HMDData.Pose.vecPosition[1] += 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_NUMPAD2))
			{
				m_HMDData.Pose.vecPosition[1] -= 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_NUMPAD9))
			{
				m_HMDData.Pose.vecPosition[2] -= 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_NUMPAD1))
			{
				m_HMDData.Pose.vecPosition[2] += 0.01;
				m_KeyDown = true;
			}
			else if (VKD(VK_NUMPAD5))
			{
				m_HMDData.Pose.vecPosition[0] = 0;
				m_HMDData.Pose.vecPosition[1] = 0;
				m_HMDData.Pose.vecPosition[2] = 0;
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
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
	{
		if (m_HMDData.PoseUpdated)
		{
			pose = m_HMDData.Pose;
			m_HMDData.PoseUpdated = false;
		}
		ReleaseMutex(m_HMDData.hPoseLock);
	}
	if (pose.poseIsValid)
		m_pDriverHost->TrackedDevicePoseUpdated(m_unObjectId, pose);
}

void CTrackedHMD::PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler)
{
	if (pData->Source != 0)
		return;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
	{
		m_HMDData.LastState = *pData;
		auto euler = Quaternion(m_HMDData.LastState.Rotation).ToEuler();
		euler.v[0] = euler.v[0] + pCenterEuler->v[0];
		euler.v[1] = euler.v[1] + pCenterEuler->v[1];
		euler.v[2] = euler.v[2] + pCenterEuler->v[2];
		m_HMDData.Pose.qRotation = Quaternion::FromEuler(euler).UnitQuaternion();
		m_HMDData.PoseUpdated = true;
		ReleaseMutex(m_HMDData.hPoseLock);
	}
}