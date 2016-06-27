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

	ZeroMemory(&m_Camera, sizeof(m_Camera));
	m_Camera.hLock = CreateMutex(nullptr, FALSE, L"CameraLock");

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

	m_HMDData.Pose.willDriftInYaw = false;
	m_HMDData.Pose.shouldApplyHeadModel = false;
	m_HMDData.Pose.deviceIsConnected = true;
	m_HMDData.Pose.poseIsValid = true;
	m_HMDData.Pose.result = ETrackingResult::TrackingResult_Running_OK;
	m_HMDData.Pose.qRotation = Quaternion(1, 0, 0, 0);
	m_HMDData.Pose.qWorldFromDriverRotation = Quaternion(1, 0, 0, 0);
	m_HMDData.Pose.qDriverFromHeadRotation = Quaternion(1, 0, 0, 0);
	m_HMDData.Pose.vecDriverFromHeadTranslation[2] = -2;
	m_HMDData.Pose.vecDriverFromHeadTranslation[2] = -0.05;
	m_HMDData.Pose.poseTimeOffset = -0.032f;

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

	char desiredCamera[128] = { 0 };
	m_pSettings->GetString("driver_customhmd", "camera", desiredCamera, sizeof(desiredCamera), "USB HD Camera");
	m_Camera.Options.Name = desiredCamera;
	m_Camera.Options.Width = 320;
	m_Camera.Options.Height = 240;
	m_Camera.Options.MediaFormat = MFVideoFormat_NV12;
	m_Camera.Options.pfCallback = CameraFrameUpdateCallback;
	m_Camera.Options.pUserData = this;

	//one time setup to determine buffersize
	m_Camera.Options.Setup();

	m_pDriverHost->TrackedDeviceAdded(Prop_SerialNumber.c_str());
}

CTrackedHMD::~CTrackedHMD()
{	
	CloseHandle(m_HMDData.hPoseLock); m_HMDData.hPoseLock = nullptr;	
	m_Camera.Destroy();
}

EVRInitError CTrackedHMD::Activate(uint32_t unObjectId)
{
	_LOG(__FUNCTION__" idx: %d", unObjectId);
	m_unObjectId = unObjectId;
	m_HMDData.IPDValue = m_pSettings->GetFloat("driver_customhmd", "IPD", 0.05f);
	return VRInitError_None;
}

void CTrackedHMD::Deactivate()
{
	_LOG(__FUNCTION__);
	m_unObjectId = k_unTrackedDeviceIndexInvalid;
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
	case Prop_CameraCompatibilityMode_Int32:
		SET_ERROR(TrackedProp_Success);
		return CAMERA_COMPAT_MODE_ISO_30FPS;
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
		return 1048581;
	case Prop_DisplayFirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 2097432;
	case Prop_DisplayHardwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 18;
	case Prop_FirmwareVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1462663157;
	case Prop_HardwareRevision_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 2147614720;
	case Prop_FPGAVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 262;
	case Prop_DongleVersion_Uint64:
		SET_ERROR(TrackedProp_Success);
		return 1461100729;
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

/*
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

*/

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
				auto euler = Quaternion((float *)&m_HMDData.LastState.RotPos.Rotation).ToEuler();
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

void CTrackedHMD::PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos)
{
	if ((pData->RotPos.Header.Type & HMD_ROTPOSDATA) != HMD_ROTPOSDATA)
		return;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
	{
		m_HMDData.LastState = *pData;		
		auto euler = Quaternion((float *)&m_HMDData.LastState.RotPos.Rotation).ToEuler();
		euler.v[0] = euler.v[0] + pCenterEuler->v[0];
		euler.v[1] = euler.v[1] + pCenterEuler->v[1];
		euler.v[2] = euler.v[2] + pCenterEuler->v[2];
		m_HMDData.Pose.qRotation = Quaternion::FromEuler(euler).UnitQuaternion();
		m_HMDData.PoseUpdated = true;
		memcpy(pRelativePos, m_HMDData.Pose.vecPosition, sizeof(HmdVector3d_t)); //to server driver
		ReleaseMutex(m_HMDData.hPoseLock);
	}

}


#define FRAME_BUFFER_COUNT 1


bool CTrackedHMD::HasCamera()
{
	_LOG(__FUNCTION__" returning 1");
	return true;
}

bool CTrackedHMD::GetCameraFirmwareDescription(char *pBuffer, uint32_t nBufferLen)
{
	_LOG(__FUNCTION__);
	strncpy_s(pBuffer, 14, m_Camera.Options.Name.c_str(), nBufferLen);
	return true;
}

bool CTrackedHMD::GetCameraFrameDimensions(ECameraVideoStreamFormat nVideoStreamFormat, uint32_t *pWidth, uint32_t *pHeight)
{
	_LOG(__FUNCTION__" fmt: %d", nVideoStreamFormat);
	//if (nVideoStreamFormat == CVS_FORMAT_RGB24)
	{
		if (pWidth) *pWidth = m_Camera.Options.Width;
		if (pHeight) *pHeight = m_Camera.Options.Height;
		return true;
	}
	return false;
}

bool CTrackedHMD::GetCameraFrameBufferingRequirements(int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize)
{
	_LOG(__FUNCTION__);
	*pDefaultFrameQueueSize = FRAME_BUFFER_COUNT;
	*pFrameBufferDataSize = m_Camera.Options.BufferSize;
	return true;
}

bool CTrackedHMD::SetCameraFrameBuffering(int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize)
{
	auto ppBuffers = (CameraVideoStreamFrame_t**)ppFrameBuffers;
	auto pFirstBuffer = ppBuffers[0];
	_LOG(__FUNCTION__" fc: %d, ds: %d, pi: %p", nFrameBufferCount, nFrameBufferDataSize, pFirstBuffer->m_pImageData);
	m_Camera.pFrameBuffer = pFirstBuffer;	
	m_Camera.Options.pCaptureBuffer = (pFirstBuffer + 1);
	return true;
}

bool CTrackedHMD::SetCameraVideoStreamFormat(ECameraVideoStreamFormat nVideoStreamFormat)
{
	_LOG(__FUNCTION__" fmt: %d", nVideoStreamFormat);
	if (CVS_FORMAT_NV12 != nVideoStreamFormat)
		return false;
	m_Camera.StreamFormat = nVideoStreamFormat;
	return true;
}

ECameraVideoStreamFormat CTrackedHMD::GetCameraVideoStreamFormat()
{
	_LOG(__FUNCTION__);
	m_Camera.StreamFormat = CVS_FORMAT_NV12;
	return m_Camera.StreamFormat;
}

float CTrackedHMD::GetVideoStreamElapsedTime()
{
	_LOG(__FUNCTION__);
	return (float)(GetTickCount() - m_Camera.StartTime) / 1000.0f;
}

const CameraVideoStreamFrame_t *CTrackedHMD::GetVideoStreamFrame()
{
	m_Camera.CallbackCount++;
	if (m_Camera.CallbackCount > m_Camera.SetupFrame.m_nBufferCount)
		return nullptr;
	//_LOG(__FUNCTION__" sf: %p, img: %p, crc: %d", m_HMDData.Camera.pCallbackStreamFrame, m_HMDData.Camera.pCallbackStreamFrame->m_pImageData, crc);
	return m_Camera.pFrameBuffer + m_Camera.SetupFrame.m_nBufferIndex;
}

void CTrackedHMD::ReleaseVideoStreamFrame(const CameraVideoStreamFrame_t *pFrameImage)
{
	//nothing to do here
	//_LOG(__FUNCTION__" sf: %p, img: %p, crc: %d", pFrameImage, pFrameImage->m_pImageData, crc);
}

bool CTrackedHMD::SetAutoExposure(bool bEnable)
{
	_LOG(__FUNCTION__" en: %d", bEnable);
	return true;
}

bool CTrackedHMD::GetCameraDistortion(float flInputU, float flInputV, float *pflOutputU, float *pflOutputV)
{
	//_LOG(__FUNCTION__" iu: %f, iv: %f ou: %f, ov: %f", flInputU, flInputV, *pflOutputU, *pflOutputV);
	*pflOutputU = flInputU;
	*pflOutputV = flInputV;
	return true;
}

bool CTrackedHMD::GetCameraProjection(float flWidthPixels, float flHeightPixels, float flZNear, float flZFar, HmdMatrix44_t *pProjection)
{
	_LOG(__FUNCTION__" w: %f, h: %f, n: %f, f: %f", flWidthPixels, flHeightPixels, flZNear, flZFar);
	Quaternion::HmdMatrix_SetIdentity(pProjection);
	float aspect = (float)m_Camera.Options.Height / (float)m_Camera.Options.Width; //  1 / tan(angleOfView * 0.5 * M_PI / 180);
	pProjection->m[0][0] = 0.15f * aspect; // scale the x coordinates of the projected point 
	pProjection->m[1][1] = 0.15f; // scale the y coordinates of the projected point 
	pProjection->m[2][2] = -flZFar / (flZFar - flZNear); // used to remap z to [0,1] 
	pProjection->m[3][2] = -flZFar * flZNear / (flZFar - flZNear); // used to remap z [0,1] 
	pProjection->m[2][3] = -1; // set w = -z 
	pProjection->m[3][3] = 0;
	return true;
}

bool CTrackedHMD::GetRecommendedCameraUndistortion(uint32_t *pUndistortionWidthPixels, uint32_t *pUndistortionHeightPixels)
{
	_LOG(__FUNCTION__);
	*pUndistortionWidthPixels = m_Camera.Options.Width;
	*pUndistortionHeightPixels = m_Camera.Options.Height;
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

bool CTrackedHMD::SetCameraVideoSinkCallback(ICameraVideoSinkCallback *pCameraVideoSinkCallback)
{
	_LOG(__FUNCTION__" cb: %p", pCameraVideoSinkCallback);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_Camera.hLock, INFINITE))
	{
		m_Camera.pfCallback = pCameraVideoSinkCallback;
		ReleaseMutex(m_Camera.hLock);
	}
	return true;
}

bool CTrackedHMD::GetCameraCompatibilityMode(ECameraCompatibilityMode *pCameraCompatibilityMode)
{
	_LOG(__FUNCTION__);
	*pCameraCompatibilityMode = CAMERA_COMPAT_MODE_ISO_30FPS;
	return true;
}

bool CTrackedHMD::SetCameraCompatibilityMode(ECameraCompatibilityMode nCameraCompatibilityMode)
{
	_LOG(__FUNCTION__" cm: %d", nCameraCompatibilityMode);
	if (nCameraCompatibilityMode == CAMERA_COMPAT_MODE_ISO_30FPS) return true;
	return false;
}

bool CTrackedHMD::GetCameraFrameBounds(EVRTrackedCameraFrameType eFrameType, uint32_t *pLeft, uint32_t *pTop, uint32_t *pWidth, uint32_t *pHeight)
{
	_LOG(__FUNCTION__" ft: %d", eFrameType);
	auto w = m_HMDData.ScreenWidth; // FRAME_WIDTH; // m_HMDData.ScreenWidth / 2;
	auto h = (m_HMDData.ScreenHeight - 30) / 2; // FRAME_HEIGHT;//(m_HMDData.ScreenHeight - 30) / 4;
	if (pLeft) *pLeft = 0; // m_HMDData.PosX + (w / 2);
	if (pTop) *pTop = 0; // m_HMDData.PosY + (h / 2);
	if (pWidth) *pWidth = w;
	if (pHeight) *pHeight = h;
	return true;
}

bool CTrackedHMD::GetCameraIntrinsics(EVRTrackedCameraFrameType eFrameType, HmdVector2_t *pFocalLength, HmdVector2_t *pCenter)
{
	_LOG(__FUNCTION__" ft: %d", eFrameType);
	return false;
}

void CTrackedHMD::SetupCamera()
{
	_LOG(__FUNCTION__);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_Camera.hLock, INFINITE))
	{
		if (!m_Camera.pCaptureDevice)		
			m_Camera.pCaptureDevice = CCaptureDevice::GetCaptureDevice(&m_Camera.Options);
		if (m_Camera.pCaptureDevice)
		{			
			m_Camera.SetupFrame.m_nBufferCount = FRAME_BUFFER_COUNT;
			m_Camera.SetupFrame.m_nBufferIndex = 0;
			m_Camera.SetupFrame.m_nWidth = m_Camera.Options.Width;
			m_Camera.SetupFrame.m_nHeight = m_Camera.Options.Height;			
			m_Camera.SetupFrame.m_nStreamFormat = ECameraVideoStreamFormat::CVS_FORMAT_NV12;
			m_Camera.SetupFrame.m_nFrameSequence = 0;
			m_Camera.SetupFrame.m_StandingTrackedDevicePose.bDeviceIsConnected = true;
			m_Camera.SetupFrame.m_StandingTrackedDevicePose.bPoseIsValid = true;
			m_Camera.SetupFrame.m_StandingTrackedDevicePose.eTrackingResult = TrackingResult_Running_OK;
			m_Camera.SetupFrame.m_nImageDataSize = m_Camera.Options.BufferSize;
			m_Camera.SetupFrame.m_pImageData = 0; // (uint64_t)m_Camera.Options.pCaptureBuffer;

			Quaternion::HmdMatrix_SetIdentity(&m_Camera.SetupFrame.m_StandingTrackedDevicePose.mDeviceToAbsoluteTracking);
		}
		if (m_Camera.pFrameBuffer)
		{
			m_Camera.SetupFrame.m_pImageData = (uint64_t)(((char *)m_Camera.pFrameBuffer) + sizeof(CameraVideoStreamFrame_t));
			*m_Camera.pFrameBuffer = m_Camera.SetupFrame;
		}
		ReleaseMutex(m_Camera.hLock);
	}
}

bool CTrackedHMD::StartVideoStream()
{
	_LOG(__FUNCTION__);
	if (!m_Camera.Options.Setup())
		return false;
	SetupCamera();
	m_Camera.StartTime = m_Camera.LastFrameTime = GetTickCount();
	if (m_Camera.pCaptureDevice)
		return m_Camera.pCaptureDevice->Start();
	return false;
}

void CTrackedHMD::StopVideoStream()
{
	_LOG(__FUNCTION__);
	if (m_Camera.pCaptureDevice)
		m_Camera.pCaptureDevice->Stop();	
}

bool CTrackedHMD::IsVideoStreamActive()
{ 	
	auto result = m_Camera.pCaptureDevice && (m_Camera.pCaptureDevice->m_Status != CCaptureDevice::Stopped);
	_LOG(__FUNCTION__" returning %d", result);
	return result;
}

bool CTrackedHMD::PauseVideoStream()
{
	_LOG(__FUNCTION__);
	if (m_Camera.pCaptureDevice)
		return m_Camera.pCaptureDevice->Pause();
	return false;
}

bool CTrackedHMD::ResumeVideoStream()
{
	_LOG(__FUNCTION__);
	if (m_Camera.pCaptureDevice)
		return m_Camera.pCaptureDevice->Resume();
	return false;
}

bool CTrackedHMD::IsVideoStreamPaused()
{
	auto result = m_Camera.pCaptureDevice && (m_Camera.pCaptureDevice->m_Status == CCaptureDevice::Paused);
	_LOG(__FUNCTION__" returning %d", result);
	return result;
}

void CTrackedHMD::CameraFrameUpdateCallback(CCaptureDevice *pDevice, void *pUserData)
{
	auto pThis = (CTrackedHMD *)pUserData;
	pThis->OnCameraFrameUpdate();
}

void CTrackedHMD::OnCameraFrameUpdate()
{
	_LOG(__FUNCTION__);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_Camera.hLock, INFINITE))
	{
		if (m_Camera.pfCallback && m_Camera.pFrameBuffer)
		{
			DWORD currTick = GetTickCount();
			auto diff = currTick - m_Camera.LastFrameTime;
			if (diff <= 0)
			{
				//prevent null div
				diff = 1;
				currTick = m_Camera.LastFrameTime + 1;
			}
			//_LOG("Camera callback %d, %d after %d ms.", m_HMDData.Camera.ActiveStreamFrame.m_nFrameSequence, m_HMDData.Camera.CallbackCount, diff);
			*m_Camera.pFrameBuffer = m_Camera.SetupFrame;			
			m_Camera.pFrameBuffer->m_flFrameElapsedTime = (currTick - m_Camera.StartTime) / 1000.0;
			m_Camera.pFrameBuffer->m_flFrameDeliveryRate = 1000.0 / diff;
			m_Camera.pFrameBuffer->m_flFrameCaptureTime_DriverAbsolute = currTick - m_Camera.StartTime;
			m_Camera.pFrameBuffer->m_nFrameCaptureTicks_ServerAbsolute = currTick - m_Camera.StartTime;			
			m_Camera.pFrameBuffer->m_nExposureTime = 1000 / diff;
			m_Camera.pFrameBuffer->m_nISPReferenceTimeStamp = m_Camera.StartTime;
			m_Camera.pFrameBuffer->m_nISPFrameTimeStamp = currTick;
			m_Camera.pFrameBuffer->m_StandingTrackedDevicePose.bDeviceIsConnected = true;
			m_Camera.pFrameBuffer->m_StandingTrackedDevicePose.bPoseIsValid = true;
			m_Camera.pFrameBuffer->m_StandingTrackedDevicePose.eTrackingResult = TrackingResult_Running_OK;
			m_Camera.pFrameBuffer->m_StandingTrackedDevicePose.vAngularVelocity.v[0] = 2;

			m_Camera.CallbackCount = 0;
			m_Camera.LastFrameTime = currTick;

			//memcpy(m_HMDData.Camera.ActiveStreamFrame.m_pImageData, m_HMDData.Camera.CaptureFrame.mTargetBuf, m_HMDData.Camera.ActiveStreamFrame.m_nImageDataSize);
			//if (*pMediaFormat == MFVideoFormat_YUY2)
			//	YUY2toNV12((uint8_t *)m_HMDData.Camera.pCaptureDevice->m_pCaptureBuffer, (uint8_t *)m_HMDData.Camera.ActiveStreamFrame.m_pImageData, FRAME_WIDTH, FRAME_HEIGHT, stride, FRAME_WIDTH);
			//else if (*pMediaFormat == MFVideoFormat_RGB24)
			//	RGB24toNV12((uint8_t *)m_HMDData.Camera.pCaptureDevice->m_pCaptureBuffer, (uint8_t *)m_HMDData.Camera.ActiveStreamFrame.m_pImageData, FRAME_WIDTH, FRAME_HEIGHT, stride, FRAME_WIDTH);

			//memcpy(m_Camera.pCallbackStreamFrame + m_Camera.ActiveStreamFrame.m_nBufferIndex, &m_Camera.ActiveStreamFrame, sizeof(CameraVideoStreamFrame_t));
			//memcpy(((char *)m_Camera.pFrameBuffer) + sizeof(CameraVideoStreamFrame_t), (void *)m_Camera.pFrameBuffer->m_pImageData, m_Camera.Options.BufferSize);

			/*
			char fn[MAX_PATH];
			sprintf_s(fn, "D:\\XXX\\Frame%d.yuv", m_Camera.pFrameBuffer->m_nFrameSequence);
			FILE *fp = _fsopen(fn, "wb", _SH_DENYNO);
			if (fp)
			{
				fwrite((const void *)m_Camera.pFrameBuffer->m_pImageData, 1, m_Camera.pFrameBuffer->m_nImageDataSize, fp);
				fclose(fp);
			}
			*/

			m_Camera.pfCallback->OnCameraVideoSinkCallback();

			m_Camera.SetupFrame.m_nFrameSequence++;
			m_Camera.SetupFrame.m_nBufferIndex = (m_Camera.SetupFrame.m_nBufferIndex + 1) % FRAME_BUFFER_COUNT;
		}
		ReleaseMutex(m_Camera.hLock);
	}
}
