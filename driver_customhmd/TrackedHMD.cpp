#include <process.h>
#include "TrackedHMD.h"
#include <mfapi.h>

CTrackedHMD::CTrackedHMD(std::string displayName, CServerDriver *pServer) : CTrackedDevice(displayName, pServer)
{
	Prop_TrackingSystemName = "Sony HMZ-T2 HMD";
	Prop_ModelNumber = "HMZ-T2";
	Prop_SerialNumber = "HMD-1244244";
	Prop_RenderModelName = "generic_hmd";
	Prop_ManufacturerName = "Sony";
	Prop_AllWirelessDongleDescriptions = "HMD-None";
	Prop_ConnectedWirelessDongle = "HMD-None";
	Prop_Firmware_ProgrammingTarget = "HMD-Multi";

	ZeroMemory(&m_HMDData, sizeof(m_HMDData));
	m_HMDData.Camera.Index = -1;

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

	m_HMDData.Pose.willDriftInYaw = false;
	m_HMDData.Pose.shouldApplyHeadModel = false;
	m_HMDData.Pose.deviceIsConnected = true;
	m_HMDData.Pose.poseIsValid = true;
	m_HMDData.Pose.result = ETrackingResult::TrackingResult_Running_OK;
	m_HMDData.Pose.qRotation = Quaternion(1, 0, 0, 0);
	m_HMDData.Pose.qWorldFromDriverRotation = Quaternion(1, 0, 0, 0);
	m_HMDData.Pose.qDriverFromHeadRotation = Quaternion(1, 0, 0, 0);

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
			_LOG("Using model %S for detection...", m_HMDData.Model);
		}

		m_HMDData.SuperSample = m_pSettings->GetFloat("driver_customhmd", "supersample", 1.0f);
	}

	m_HMDData.Logger = m_pLog;

	if (!m_HMDData.DirectMode)
	{

		_LOG("HMD: Enumerating monitors...");
		EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&m_HMDData);
		_LOG("HMD: Monitor detection finished.");
	}
	m_pDriverHost->TrackedDeviceAdded(Prop_SerialNumber.c_str());
}

CTrackedHMD::~CTrackedHMD()
{
	//Deactivate();		
	CloseHandle(m_HMDData.hPoseLock);
}

EVRInitError CTrackedHMD::Activate(uint32_t unObjectId)
{
	CoInitialize(NULL);
	_LOG(__FUNCTION__" idx: %d", unObjectId);
	InitCamera();
	m_unObjectId = unObjectId;
	m_HMDData.IPDValue = m_pSettings->GetFloat("driver_customhmd", "IPD", 0.05f);
	return VRInitError_None;
}

void CTrackedHMD::Deactivate()
{
	_LOG(__FUNCTION__);
	m_unObjectId = k_unTrackedDeviceIndexInvalid;
	DeinitCamrea();
	CoUninitialize();
	//	TRACE(__FUNCTIONW__);
}

void *CTrackedHMD::GetComponent(const char *pchComponentNameAndVersion)
{
	_LOG(__FUNCTION__" %s", pchComponentNameAndVersion);
	if (!_stricmp(pchComponentNameAndVersion, IVRDisplayComponent_Version))
	{
		return (IVRDisplayComponent*)this;
	}
	else if (!_stricmp(pchComponentNameAndVersion, IVRCameraComponent_Version))
	{
		return (IVRCameraComponent*)this;
	}

	// override this to add a component to a driver
	return nullptr;
}

void CTrackedHMD::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
	_LOG(__FUNCTION__" %s", pchRequest);
}

void CTrackedHMD::GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
	*pnX = m_HMDData.PosX;
	*pnY = m_HMDData.PosY;
	*pnWidth = m_HMDData.ScreenWidth;
	*pnHeight = m_HMDData.ScreenHeight;
	_LOG(__FUNCTION__" x: %d, y: %d, w: %d, h: %d", *pnX, *pnY, *pnWidth, *pnHeight);
}

bool CTrackedHMD::IsDisplayOnDesktop()
{
	_LOG(__FUNCTION__" returning %d", !m_HMDData.DirectMode);
	return !m_HMDData.DirectMode;
}

bool CTrackedHMD::IsDisplayRealDisplay()
{
	_LOG(__FUNCTION__" returning 1");
	return true;
}

void CTrackedHMD::GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight)
{
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
	_LOG(__FUNCTION__" w: %d, h: %d", *pnWidth, *pnHeight);
}

void CTrackedHMD::GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
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
	_LOG(__FUNCTION__" Eye: %d, x: %d, y: %d, w: %d, h: %d", eEye, *pnX, *pnY, *pnWidth, *pnHeight);
}

void CTrackedHMD::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom)
{
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
	_LOG(__FUNCTION__" Eye: %d, l: %f, r: %f, t: %f, b: %f", eEye, *pfLeft, *pfRight, *pfTop, *pfBottom);
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
		return 8590262285;
	case Prop_PreviousUniverseId_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 0;
	case Prop_DisplayFPGAVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 57;
	case Prop_AudioFirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 3;
	case Prop_DisplayBootloaderVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 0;
	case Prop_DisplayFirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 2097432;
	case Prop_DisplayHardwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 18;
	case Prop_FirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1454541759;
	case Prop_HardwareRevision_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 2147614720;
	case Prop_FPGAVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 523;
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
		return "HeadSet Camera";
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
	_LOG(__FUNCTION__" id: %d, fmt: %d, w: %d, h: %d", unPid, unFormat, unWidth, unHeight);
}

void CTrackedHMD::DestroySwapTextureSet(void *pSharedTextureHandle)
{
	_LOG(__FUNCTION__);
}

void CTrackedHMD::DestroyAllSwapTextureSets(uint32_t unPid)
{
	_LOG(__FUNCTION__" id: %d", unPid);
}

void CTrackedHMD::GetNextSwapTextureSetIndex(void *pSharedTextureHandles[2], uint32_t(*pIndices)[2])
{
	_LOG(__FUNCTION__);
}

void CTrackedHMD::SubmitLayer(void *pSharedTextureHandles[2], const VRTextureBounds_t(&bounds)[2], const HmdMatrix34_t *pPose)
{
	_LOG(__FUNCTION__);
}

void CTrackedHMD::Present(void *hSyncTexture)
{
	_LOG(__FUNCTION__);
}

void CTrackedHMD::PowerOff()
{
	_LOG(__FUNCTION__);
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
				m_pDriverHost->PhysicalIpdSet(m_unObjectId, m_HMDData.IPDValue);
				m_pSettings->SetFloat("driver_customhmd", "IPD", m_HMDData.IPDValue);
				m_pSettings->Sync(true);
				m_KeyDown = true;
			}
			else if (VKD(VK_SUBTRACT))
			{
				m_HMDData.IPDValue -= step;
				m_pDriverHost->PhysicalIpdSet(m_unObjectId, m_HMDData.IPDValue);
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



bool CTrackedHMD::HasCamera()
{
	_LOG(__FUNCTION__" returning 1");
	return true;
}

bool CTrackedHMD::GetCameraFirmwareDescription(char *pBuffer, uint32_t nBufferLen)
{
	_LOG(__FUNCTION__);
	strncpy_s(pBuffer, 14, "HeadSet Camera", nBufferLen);
	return true;
}

bool CTrackedHMD::GetCameraFrameDimensions(vr::ECameraVideoStreamFormat nVideoStreamFormat, uint32_t *pWidth, uint32_t *pHeight)
{
	_LOG(__FUNCTION__" fmt: %d", nVideoStreamFormat);
	//if (nVideoStreamFormat == CVS_FORMAT_RGB24)
	{
		if (pWidth) *pWidth = 320;
		if (pHeight) *pHeight = 240;
		return true;
	}
	return false;
}

bool CTrackedHMD::GetCameraFrameBufferingRequirements(int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize)
{
	_LOG(__FUNCTION__);
	*pDefaultFrameQueueSize = 1;
	*pFrameBufferDataSize = 320 * 240 * sizeof(int);
	return true;
}

bool CTrackedHMD::SetCameraFrameBuffering(int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize)
{
	_LOG(__FUNCTION__" fc: %d, ds: %d, ptr: %p pptr:%p sf: %p", nFrameBufferCount, nFrameBufferDataSize, *ppFrameBuffers, ppFrameBuffers, m_HMDData.Camera.pCallbackStreamFrame);
	m_HMDData.Camera.pCallbackStreamFrame = (CameraVideoStreamFrame_t*)*ppFrameBuffers;
	return true;
}

bool CTrackedHMD::SetCameraVideoStreamFormat(vr::ECameraVideoStreamFormat nVideoStreamFormat)
{
	_LOG(__FUNCTION__" fmt: %d", nVideoStreamFormat);
	m_HMDData.Camera.StreamFormat = nVideoStreamFormat;
	return true;
}

vr::ECameraVideoStreamFormat CTrackedHMD::GetCameraVideoStreamFormat()
{
	_LOG(__FUNCTION__);
	m_HMDData.Camera.StreamFormat = CVS_FORMAT_NV12;
	return m_HMDData.Camera.StreamFormat;
}

bool CTrackedHMD::StartVideoStream()
{
	_LOG(__FUNCTION__);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.Camera.hLock, INFINITE))
	{
		if (!m_HMDData.Camera.IsActive && m_HMDData.Camera.Index > -1)
		{
			DWORD currTick = GetTickCount();
			m_HMDData.Camera.hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, CameraThread, this, CREATE_SUSPENDED, nullptr));
			if (m_HMDData.Camera.hThread)
			{
				m_HMDData.Camera.IsActive = true;
				m_HMDData.Camera.StartTime = currTick;
				m_HMDData.Camera.LastFrameTime = m_HMDData.Camera.StartTime;
				ResumeThread(m_HMDData.Camera.hThread);
			}
		}
		ReleaseMutex(m_HMDData.Camera.hLock);
	}
	return m_HMDData.Camera.IsActive;
}

void CTrackedHMD::StopVideoStream()
{
	_LOG(__FUNCTION__);
	HANDLE hTemp = nullptr;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.Camera.hLock, INFINITE))
	{
		if (m_HMDData.Camera.IsActive)
		{
			m_HMDData.Camera.IsActive = false;
			if (m_HMDData.Camera.hThread)
			{
				hTemp = m_HMDData.Camera.hThread;
				m_HMDData.Camera.hThread = nullptr;
			}
		}
		ReleaseMutex(m_HMDData.Camera.hLock);
	}

	if (hTemp)
	{
		WaitForSingleObject(hTemp, INFINITE);
		CloseHandle(hTemp);
	}
}

bool CTrackedHMD::IsVideoStreamActive()
{
	_LOG(__FUNCTION__" returning %d", m_HMDData.Camera.IsActive);
	return m_HMDData.Camera.IsActive;
}

float CTrackedHMD::GetVideoStreamElapsedTime()
{
	_LOG(__FUNCTION__);
	return (float)(GetTickCount() - m_HMDData.Camera.StartTime) / 1000.0f;
}

const vr::CameraVideoStreamFrame_t *CTrackedHMD::GetVideoStreamFrame()
{
	m_HMDData.Camera.CallbackCount++;
	if (m_HMDData.Camera.CallbackCount > 1)
		return nullptr;

	_LOG(__FUNCTION__" sf: %p", m_HMDData.Camera.pCallbackStreamFrame);
	return m_HMDData.Camera.pCallbackStreamFrame;
}

void CTrackedHMD::ReleaseVideoStreamFrame(const vr::CameraVideoStreamFrame_t *pFrameImage)
{
	_LOG(__FUNCTION__" %p", pFrameImage);
	//_LOG(__FUNCTION__);
	//free(pFrameImage->m_pImageData);
	//delete pFrameImage;

	//if (pFrameImage)
	//	delete pFrameImage;
}

bool CTrackedHMD::SetAutoExposure(bool bEnable)
{
	_LOG(__FUNCTION__" en: %d", bEnable);
	return true;
}

bool CTrackedHMD::PauseVideoStream()
{
	_LOG(__FUNCTION__);
	if (m_HMDData.Camera.IsActive)
	{
		StopVideoStream();
		return true;
	}
	return false;
}

bool CTrackedHMD::ResumeVideoStream()
{
	_LOG(__FUNCTION__);
	return StartVideoStream();
}

bool CTrackedHMD::IsVideoStreamPaused()
{
	_LOG(__FUNCTION__" returning %d", !m_HMDData.Camera.IsActive);
	return !m_HMDData.Camera.IsActive;;
}

bool CTrackedHMD::GetCameraDistortion(float flInputU, float flInputV, float *pflOutputU, float *pflOutputV)
{	
	//_LOG(__FUNCTION__" iu: %f, iv: %f ou: %f, ov: %f", flInputU, flInputV, *pflOutputU, *pflOutputV);
	*pflOutputU = flInputU;
	*pflOutputV = flInputV;
	return true;
}

bool CTrackedHMD::GetCameraProjection(float flWidthPixels, float flHeightPixels, float flZNear, float flZFar, vr::HmdMatrix44_t *pProjection)
{
	_LOG(__FUNCTION__" w: %f, h: %f, n: %f, f: %f", flWidthPixels, flHeightPixels, flZNear, flZFar);

	Quaternion::HmdMatrix_SetIdentity(pProjection);

	float r = 0, l = flWidthPixels, b = flHeightPixels, t = 0, n = flZNear, f = flZFar;

	pProjection->m[0][0] = 2 / (r - l);
	pProjection->m[0][1] = 0;
	pProjection->m[0][2] = 0;
	pProjection->m[0][3] = 0;
	pProjection->m[1][0] = 0;
	pProjection->m[1][1] = 2 / (t - b);
	pProjection->m[1][2] = 0;
	pProjection->m[1][3] = 0;
	pProjection->m[2][0] = 0;
	pProjection->m[2][1] = 0;
	pProjection->m[2][2] = -2 / (f - n);
	pProjection->m[2][3] = 0;
	pProjection->m[3][0] = -(r + l) / (r - l);
	pProjection->m[3][1] = -(t + b) / (t - b);
	pProjection->m[3][2] = -(f + n) / (f - n);
	pProjection->m[3][3] = 1;

	//pProjection->m[0][0] = 2 * flZNear / flWidthPixels;
	//pProjection->m[0][2] = 1.0f;
	//pProjection->m[1][1] = 2 * flZNear / flHeightPixels;
	//pProjection->m[1][2] = 1.0f;
	//pProjection->m[2][2] = -(flZFar + flZNear) / (flZFar - flZNear);
	//pProjection->m[2][3] = -(2 * flZNear * flZFar) / (flZFar - flZNear);
	//pProjection->m[3][2] = -1.0f;
	//pProjection->m[3][3] = 0.0f;
	return true;
}

bool CTrackedHMD::GetRecommendedCameraUndistortion(uint32_t *pUndistortionWidthPixels, uint32_t *pUndistortionHeightPixels)
{
	_LOG(__FUNCTION__);
	*pUndistortionWidthPixels = 320;
	*pUndistortionHeightPixels = 240;
	return true;
}

bool CTrackedHMD::SetCameraUndistortion(uint32_t nUndistortionWidthPixels, uint32_t nUndistortionHeightPixels)
{
	_LOG(__FUNCTION__" w: %d, h: %d", nUndistortionWidthPixels, nUndistortionHeightPixels);
	return true;
}

bool CTrackedHMD::GetCameraFirmwareVersion(uint64_t *pFirmwareVersion)
{
	_LOG(__FUNCTION__);
	*pFirmwareVersion = 8590262285;
	return true;
}

bool CTrackedHMD::SetFrameRate(int nISPFrameRate, int nSensorFrameRate)
{
	_LOG(__FUNCTION__" ifr: %d, sfr: %d ", nISPFrameRate, nSensorFrameRate);
	return true;
}

bool CTrackedHMD::SetCameraVideoSinkCallback(vr::ICameraVideoSinkCallback *pCameraVideoSinkCallback)
{
	_LOG(__FUNCTION__" cb: %p", pCameraVideoSinkCallback);
	m_HMDData.Camera.pfCallback = pCameraVideoSinkCallback;
	return true;
}

bool CTrackedHMD::GetCameraCompatibilityMode(vr::ECameraCompatibilityMode *pCameraCompatibilityMode)
{
	_LOG(__FUNCTION__);
	*pCameraCompatibilityMode = CAMERA_COMPAT_MODE_ISO_30FPS;
	return true;
}

bool CTrackedHMD::SetCameraCompatibilityMode(vr::ECameraCompatibilityMode nCameraCompatibilityMode)
{
	_LOG(__FUNCTION__" cm: %d", nCameraCompatibilityMode);
	if (nCameraCompatibilityMode == CAMERA_COMPAT_MODE_ISO_30FPS) return true;
	return false;
}

bool CTrackedHMD::GetCameraFrameBounds(vr::EVRTrackedCameraFrameType eFrameType, uint32_t *pLeft, uint32_t *pTop, uint32_t *pWidth, uint32_t *pHeight)
{
	_LOG(__FUNCTION__" ft: %d", eFrameType);
	auto w = 320; // m_HMDData.ScreenWidth / 2;
	auto h = 240; // (m_HMDData.ScreenHeight - 30) / 4;
	*pLeft = 0; // m_HMDData.PosX + (w / 2);
	*pTop = 0; // m_HMDData.PosY + (h / 2);
	*pWidth = w;
	*pHeight = h;
	return true;
}

bool CTrackedHMD::GetCameraIntrinsics(vr::EVRTrackedCameraFrameType eFrameType, HmdVector2_t *pFocalLength, HmdVector2_t *pCenter)
{
	_LOG(__FUNCTION__" ft: %d", eFrameType);
	return false;
}

bool CTrackedHMD::InitCamera()
{

	_LOG(__FUNCTION__);
	if (m_HMDData.Camera.Index != -1)
		return true;

	int devices = countCaptureDevices();
	if (devices <= 0)
	{
		_LOG("HMD: Camera not found!");
		return false;
	}

	char desiredCamera[128] = { 0 };
	char cameraName[128] = { 0 };
	m_pSettings->GetString("driver_customhmd", "camera", desiredCamera, sizeof(desiredCamera), "USB HD Camera");

	for (int i = 0; i < devices; i++)
	{
		cameraName[0] = 0;
		getCaptureDeviceName(i, cameraName, sizeof(cameraName));
		if (!strcmp(cameraName, desiredCamera))
		{
			_LOG("HMD: Camera initializing...");
			m_HMDData.Camera.hLock = CreateMutex(nullptr, FALSE, L"CameraLock");
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.Camera.hLock, INFINITE))
			{
				m_HMDData.Camera.Index = i;
				m_HMDData.Camera.CaptureFrame.mWidth = 320;
				m_HMDData.Camera.CaptureFrame.mHeight = 240;
				m_HMDData.Camera.CaptureFrame.mTargetBuf = (int *)malloc(m_HMDData.Camera.CaptureFrame.mHeight * m_HMDData.Camera.CaptureFrame.mWidth * sizeof(int));
				m_HMDData.Camera.CaptureFrame.mTargetFormat = MFVideoFormat_NV12;
				initCaptureWithOptions(m_HMDData.Camera.Index, &m_HMDData.Camera.CaptureFrame, CAPTURE_OPTION_RAWDATA);
				m_HMDData.Camera.StartTime = GetTickCount();
				m_HMDData.Camera.ActiveStreamFrame.m_nBufferCount = 1;
				m_HMDData.Camera.ActiveStreamFrame.m_nBufferIndex = 0;
				m_HMDData.Camera.ActiveStreamFrame.m_nWidth = m_HMDData.Camera.CaptureFrame.mWidth;
				m_HMDData.Camera.ActiveStreamFrame.m_nHeight = m_HMDData.Camera.CaptureFrame.mHeight;
				m_HMDData.Camera.ActiveStreamFrame.m_nImageDataSize = m_HMDData.Camera.CaptureFrame.mHeight * m_HMDData.Camera.CaptureFrame.mWidth * sizeof(int);
				m_HMDData.Camera.StreamFormat = CVS_FORMAT_NV12;
				m_HMDData.Camera.ActiveStreamFrame.m_StandingTrackedDevicePose.bDeviceIsConnected = true;
				m_HMDData.Camera.ActiveStreamFrame.m_StandingTrackedDevicePose.bPoseIsValid = true;
				m_HMDData.Camera.ActiveStreamFrame.m_StandingTrackedDevicePose.eTrackingResult = TrackingResult_Running_OK;
				m_HMDData.Camera.ActiveStreamFrame.m_pImageData = (int *)malloc(m_HMDData.Camera.CaptureFrame.mHeight * m_HMDData.Camera.CaptureFrame.mWidth * sizeof(int));
				Quaternion::HmdMatrix_SetIdentity(&m_HMDData.Camera.ActiveStreamFrame.m_StandingTrackedDevicePose.mDeviceToAbsoluteTracking);

				ReleaseMutex(m_HMDData.Camera.hLock);
			}
			return true;
		}
	}
	return false;
}

void CTrackedHMD::DeinitCamrea()
{
	_LOG(__FUNCTION__);
	StopVideoStream();
	_LOG("HMD: Camera deinitializing...");
	deinitCapture(m_HMDData.Camera.Index);

	if (m_HMDData.Camera.hLock)
	{
		WaitForSingleObject(m_HMDData.Camera.hLock, INFINITE);
		CloseHandle(m_HMDData.Camera.hLock);
	}

	if (m_HMDData.Camera.CaptureFrame.mTargetBuf)
		free(m_HMDData.Camera.CaptureFrame.mTargetBuf);

	if (m_HMDData.Camera.ActiveStreamFrame.m_pImageData)
		free(m_HMDData.Camera.ActiveStreamFrame.m_pImageData);

	ZeroMemory(&m_HMDData.Camera, sizeof(m_HMDData.Camera));
	m_HMDData.Camera.Index = -1;
}

unsigned int WINAPI CTrackedHMD::CameraThread(void *p)
{
	auto pHmd = static_cast<CTrackedHMD *>(p);
	if (pHmd)
		pHmd->RunCamera();
	_endthreadex(0);
	return 0;

}

void CTrackedHMD::RunCamera()
{
	m_HMDData.Camera.LastFrameTime = GetTickCount();
	doCapture(m_HMDData.Camera.Index);
	bool isActive = m_HMDData.Camera.IsActive;
	while (isActive)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.Camera.hLock, INFINITE))
		{
			if (m_HMDData.Camera.pfCallback && m_HMDData.Camera.pCallbackStreamFrame && isCaptureDone(m_HMDData.Camera.Index))
			{
				DWORD currTick = GetTickCount();
				auto diff = currTick - m_HMDData.Camera.LastFrameTime;
				_LOG("Camera callback %d, %d after %d ms.", m_HMDData.Camera.ActiveStreamFrame.m_nFrameSequence, m_HMDData.Camera.CallbackCount, diff);

				m_HMDData.Camera.ActiveStreamFrame.m_flFrameElapsedTime = (currTick - m_HMDData.Camera.StartTime) / 1000.0;
				m_HMDData.Camera.ActiveStreamFrame.m_flFrameDeliveryRate = 1000.0 / diff;
				m_HMDData.Camera.ActiveStreamFrame.m_flFrameCaptureTime_DriverAbsolute = currTick - m_HMDData.Camera.StartTime;
				m_HMDData.Camera.ActiveStreamFrame.m_nFrameCaptureTicks_ServerAbsolute = currTick - m_HMDData.Camera.StartTime;

				m_HMDData.Camera.ActiveStreamFrame.m_nStreamFormat = m_HMDData.Camera.StreamFormat;

				m_HMDData.Camera.ActiveStreamFrame.m_nExposureTime = 1000 / diff;

				m_HMDData.Camera.ActiveStreamFrame.m_nISPReferenceTimeStamp = m_HMDData.Camera.StartTime;
				m_HMDData.Camera.ActiveStreamFrame.m_nISPFrameTimeStamp = currTick;

				m_HMDData.Camera.ActiveStreamFrame.m_StandingTrackedDevicePose.bDeviceIsConnected = true;
				m_HMDData.Camera.ActiveStreamFrame.m_StandingTrackedDevicePose.bPoseIsValid = true;
				m_HMDData.Camera.ActiveStreamFrame.m_StandingTrackedDevicePose.eTrackingResult = TrackingResult_Running_OK;

				m_HMDData.Camera.CallbackCount = 0;
				m_HMDData.Camera.LastFrameTime = currTick;

				//memcpy(m_HMDData.Camera.ActiveStreamFrame.m_pImageData, m_HMDData.Camera.CaptureFrame.mTargetBuf, m_HMDData.Camera.ActiveStreamFrame.m_nImageDataSize);
				YUY2oNV12((uint8_t *)m_HMDData.Camera.CaptureFrame.mTargetBuf, (uint8_t *)m_HMDData.Camera.ActiveStreamFrame.m_pImageData, 320, 240);
				m_HMDData.Camera.ActiveStreamFrame.m_nImageDataSize = 115200;


				memcpy(m_HMDData.Camera.pCallbackStreamFrame, &m_HMDData.Camera.ActiveStreamFrame, sizeof(m_HMDData.Camera.ActiveStreamFrame));

				//char fn[MAX_PATH];
				//sprintf_s(fn, "D:\\XXX\\Frame%d.yuv", m_HMDData.Camera.ActiveStreamFrame.m_nFrameSequence);
				//FILE *fp = _fsopen(fn, "wb", _SH_DENYNO);
				//if (fp)
				//{
				//	fwrite(m_HMDData.Camera.ActiveStreamFrame.m_pImageData, 1, m_HMDData.Camera.ActiveStreamFrame.m_nImageDataSize, fp);
				//	fclose(fp);
				//}
				//const char *pSrc = (const char *)m_HMDData.Camera.CaptureFrame.mTargetBuf;
				//char *pDst = (char *)m_HMDData.Camera.ActiveStreamFrame.m_pImageData;				
				//ARGBToNV12(pSrc, 320 * sizeof(int) * 8, pDst, 320/8, pDst+(320 * 240 /8), 320 /8, 320, 240);

				m_HMDData.Camera.pfCallback->OnCameraVideoSinkCallback(); 

				m_HMDData.Camera.ActiveStreamFrame.m_nFrameSequence++;
				doCapture(m_HMDData.Camera.Index);
			}
			isActive = m_HMDData.Camera.IsActive;
			ReleaseMutex(m_HMDData.Camera.hLock);
		}
		Sleep(32);
	}
}

void CTrackedHMD::YUY2oNV12(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height)
{
	/* Color space conversion from YUYV to YUV420SP */
	//TODO: change to NV12 (from YYYYYYYY UUVV to YYYYYYYY UVUV)
	int src_size = 0, out_size = 0;
	uint8_t *p;
	uint8_t *Y, *U, *V;

	src_size = (width * height) << 1;//YUY2 4:2:2
	p = inputBuffer;

	out_size = width * height * 3 / 2;//YUV 4:2:0
	Y = outputBuffer;
	U = Y + width * height;
	V = U + (width *  height >> 2);

	memset(outputBuffer, 0, out_size);

	for (int k = 0; k < height; ++k) {
		for (int j = 0; j < (width >> 1); ++j) {
			Y[j * 2] = p[4 * j];
			Y[j * 2 + 1] = p[4 * j + 2];
			if (k % 2 == 0) {
				U[j] = p[4 * j + 1];
				V[j] = p[4 * j + 3];
			}
		}
		p = p + width * 2;

		Y = Y + width;
		U = U + (width >> 2);
		V = V + (width >> 2);
	}	
}
