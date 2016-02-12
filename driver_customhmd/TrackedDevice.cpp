#include "ServerDriver.h"
#include <process.h>
#include "Serial.h"

CTrackedDevice::CTrackedDevice(std::string id, CServerDriver *pServer)
{
//	TRACE(__FUNCTIONW__);

	m_Pose = {};
	m_Id = id;
	m_hThread = nullptr;
	m_pDriverHost = pServer->driverHost_;
	m_pLog = pServer->logger_;
	m_IsRunning = false;
	m_PIDValue = 0.03f;
	m_MonData = { 0 };
	m_MonData.HMD_POSX = 0;
	m_MonData.HMD_POSY = 0;
	m_MonData.HMD_WIDTH = 1920;
	m_MonData.HMD_HEIGHT = 1080;
	m_MonData.HMD_ASPECT = (float)m_MonData.HMD_WIDTH / (float)m_MonData.HMD_HEIGHT;
	m_MonData.HMD_FREQ = 60;
	m_MonData.HMD_FOUND = false;

	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM) &m_MonData);
}

EVRInitError CTrackedDevice::Activate(uint32_t unObjectId)
{
//	TRACE(__FUNCTIONW__);
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ProcessThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hThread);
	}

	return vr::VRInitError_None;
}

unsigned int WINAPI CTrackedDevice::ProcessThread(void *p)
{
//	TRACE(__FUNCTIONW__);
	if (!p) return -1;
	auto device = static_cast<CTrackedDevice *>(p);
	if (device)
		device->Run();
	_endthreadex(0);
	return 0;
}

//#define MATH_PI (3.14159265359)

void CTrackedDevice::Run()
{
//	TRACE(__FUNCTIONW__);
	HTData htData = {};
	char *array = (char *)&htData;
	int pos = 0;

	double _yaw = 0;
	double _pitch = 0;
	double _roll = 0;

	double _yawRaw = 0;
	double _pitchRaw = 0;
	double _rollRaw = 0;

	double _yawCenter = 0;
	double _pitchCenter = 0;
	double _rollCenter = 0;
	
	CSerial *pSerial = new CSerial(std::wstring(L"COM3"), 115200);
	std::string incBuffer;

	char data;
	float step = 0.005f;
	auto keydown = false;

	while (m_IsRunning)
	{		
		if ((GetAsyncKeyState(VK_HOME) & 0x8000) && ((GetAsyncKeyState(VK_CONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				//center
				_yawCenter = _yawRaw;
				_pitchCenter = _pitchRaw;
				_rollCenter = _rollRaw;
				keydown = true;
			}
		}
		else if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && ((GetAsyncKeyState(VK_CONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				m_PIDValue += step;
				m_pDriverHost->PhysicalIpdSet(0, m_PIDValue);
				m_pLog->Log("up\n");
				keydown = true;
			}
		}
		else if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && ((GetAsyncKeyState(VK_CONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				m_PIDValue -= step;
				m_pDriverHost->PhysicalIpdSet(0, m_PIDValue);
				m_pLog->Log("down\n");
				keydown = true;
			}
		}
		else
			keydown = false;

		try
		{
			pos = 0;
			if (!pSerial->Read(&data, 1))
				continue;
			if (data != (char)0xff)
				continue;
			array[pos++] = data;

			if (!pSerial->Read(&data, 1))
				continue;
			if (data != (char)0xff)
				continue;
			array[pos++] = data;

			if (!pSerial->Read(&data, 1))
				continue;
			if (data != (char)0xff)
				continue;
			array[pos++] = data;

			if (!pSerial->Read(&data, 1))
				continue;
			if (data != (char)0xff)
				continue;
			array[pos++] = data;

			if (!pSerial->Read(&array[pos], sizeof(HTData) - sizeof(int)))
				continue;

			//deðerler radyan *10000 geliyo seriden
			_yawRaw = (double)htData.yaw / -10000.0;
			_pitchRaw = (double)htData.pitch / 10000.0;
			_rollRaw = (double)htData.roll / -10000.0;

			_yaw = _yawRaw - _yawCenter;
			_pitch = _pitchRaw - _pitchCenter;
			_roll = _rollRaw - _rollCenter;

			m_Pose.qRotation.w = 1;
			m_Pose.qRotation.x = 0;
			m_Pose.qRotation.y = 0;
			m_Pose.qRotation.z = 0;

			m_Pose.qWorldFromDriverRotation.w = 1;
			m_Pose.qWorldFromDriverRotation.x = 0;
			m_Pose.qWorldFromDriverRotation.y = 0;
			m_Pose.qWorldFromDriverRotation.z = 0;

			m_Pose.qDriverFromHeadRotation.w = 1;
			m_Pose.qDriverFromHeadRotation.x = 0;
			m_Pose.qDriverFromHeadRotation.y = 0;
			m_Pose.qDriverFromHeadRotation.z = 0;


			double num9 = -_roll * 0.5f;
			double num6 = sin(num9);
			double num5 = cos(num9);
			double num8 = -_pitch * 0.5f;
			double num4 = sin(num8);
			double num3 = cos(num8);
			double num7 = _yaw * 0.5f;
			double num2 = sin(num7);
			double num = cos(num7);
			m_Pose.qRotation.x = ((num * num4) * num5) + ((num2 * num3) * num6);
			m_Pose.qRotation.y = ((num2 * num3) * num5) - ((num * num4) * num6);
			m_Pose.qRotation.z = ((num * num3) * num6) - ((num2 * num4) * num5);
			m_Pose.qRotation.w = ((num * num3) * num5) + ((num2 * num4) * num6);

			m_Pose.poseIsValid = true;
			m_Pose.result = ETrackingResult::TrackingResult_Running_OK;
			m_Pose.willDriftInYaw = true;
			m_Pose.shouldApplyHeadModel = false;

			m_pDriverHost->TrackedDevicePoseUpdated(0, m_Pose );
		}
		catch (...)
		{
		}		
		Sleep(1);
	}

	delete pSerial;
}

void CTrackedDevice::Deactivate()
{
	m_IsRunning = false;
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}
//	TRACE(__FUNCTIONW__);
}

const char * CTrackedDevice::GetId()
{
//	TRACE(__FUNCTIONW__);
	return m_Id.c_str();
}

void CTrackedDevice::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
//	TRACE(__FUNCTIONW__);
}

void CTrackedDevice::GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
//	TRACE(__FUNCTIONW__);
	*pnX = m_MonData.HMD_POSX;
	*pnY = m_MonData.HMD_POSY;
	*pnWidth = m_MonData.HMD_WIDTH;
	*pnHeight = m_MonData.HMD_HEIGHT;
}

bool CTrackedDevice::IsDisplayOnDesktop()
{
//	TRACE(__FUNCTIONW__);
	return true;
}

bool CTrackedDevice::IsDisplayRealDisplay()
{
//	TRACE(__FUNCTIONW__);
	return true;
}

void CTrackedDevice::GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight)
{
//	TRACE(__FUNCTIONW__);
	*pnWidth = m_MonData.HMD_WIDTH;
	*pnHeight = m_MonData.HMD_HEIGHT;
}

void CTrackedDevice::GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
//	TRACE(__FUNCTIONW__);
	switch (eEye)
	{
	case EVREye::Eye_Left:
		*pnX = 0;
		*pnY = 0;
		*pnWidth = m_MonData.HMD_WIDTH / 2;
		*pnHeight = m_MonData.HMD_HEIGHT;
		break;
	case EVREye::Eye_Right:
		*pnX = 0 + (m_MonData.HMD_WIDTH / 2);
		*pnY = 0;
		*pnWidth = m_MonData.HMD_WIDTH / 2;
		*pnHeight = m_MonData.HMD_HEIGHT;
		break;
	}
}

void CTrackedDevice::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom)
{
//	TRACE(__FUNCTIONW__);
	switch (eEye)
	{
	case EVREye::Eye_Left:
		*pfLeft = -1.0f;
		*pfRight = 1.0f;
		*pfTop = -1.0f / m_MonData.HMD_ASPECT;
		*pfBottom = 1.0f / m_MonData.HMD_ASPECT;
		break;
	case EVREye::Eye_Right:
		*pfLeft = -1.0f;
		*pfRight = 1.0f;
		*pfTop = -1.0f / m_MonData.HMD_ASPECT;
		*pfBottom = 1.0f / m_MonData.HMD_ASPECT;
		break;
	}
}

DistortionCoordinates_t CTrackedDevice::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	vr::DistortionCoordinates_t coords;
	coords.rfRed[0] = fU;
	coords.rfRed[1] = fV;
	coords.rfBlue[0] = fU;
	coords.rfBlue[1] = fV;
	coords.rfGreen[0] = fU;
	coords.rfGreen[1] = fV;
	return coords;
}

TrackedDeviceDriverInfo_t CTrackedDevice::GetTrackedDeviceDriverInfo()
{
//	TRACE(__FUNCTIONW__);
	vr::TrackedDeviceDriverInfo_t info = {};
	strcpy_s(info.rchTrackingSystemId, "SONY HMZ-T2"); // TODO name of the underlying tracking system
	strcpy_s(info.rchSerialNumber, GetSerialNumber());
	strcpy_s(info.rchModelNumber, GetModelNumber());
	strcpy_s(info.rchRenderModelName, "\\\\.\\DISPLAY2");        // TODO pass this to GetRenderModel to get the mesh and texture to render this device
	info.eClass = vr::TrackedDeviceClass_HMD; // TODO adjust accordingly
	info.bDeviceIsConnected = true;           // false if user unplugs device
	info.bWillDriftInYaw = false;              // true if gyro-only tracking system
	info.bReportsTimeSinceVSync = false;
	info.fSecondsFromVsyncToPhotons = 0.0; // seconds between vsync and photons hitting wearer's eyes
	info.fDisplayFrequency = m_MonData.HMD_FREQ;         // fps of display
	return info;
}

const char * CTrackedDevice::GetModelNumber()
{
//	TRACE(__FUNCTIONW__);
	return "CUSTOM HMD";
}

const char * CTrackedDevice::GetSerialNumber()
{
//	TRACE(__FUNCTIONW__);
	return "1244244";
}

DriverPose_t CTrackedDevice::GetPose()
{
//	TRACE(__FUNCTIONW__);
	return m_Pose ;
}

bool CTrackedDevice::GetBoolTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *error)
{
	//TRACE(__FUNCTIONW__);
	const bool default_value = false;
	switch (prop) {
	case vr::Prop_ContainsProximitySensor_Bool:
		if (error)
			*error = vr::TrackedProp_Success;
		return false;
	case vr::Prop_WillDriftInYaw_Bool: // TODO
		if (error)
			*error = vr::TrackedProp_Success;
		return false;
	case vr::Prop_ReportsTimeSinceVSync_Bool: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_IsOnDesktop_Bool: // TODO
		if (error)
			*error = vr::TrackedProp_Success;
		return true;
	}

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

float CTrackedDevice::GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * error)
{
//	TRACE(__FUNCTIONW__);
	const float default_value = 0.0f;

	switch (prop) {
	case vr::Prop_SecondsFromVsyncToPhotons_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_DisplayFrequency_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_UserIpdMeters_Float:
		if (error)
			*error = vr::TrackedProp_Success;
		return m_PIDValue;
	case vr::Prop_FieldOfViewLeftDegrees_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_FieldOfViewRightDegrees_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_FieldOfViewTopDegrees_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_FieldOfViewBottomDegrees_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_TrackingRangeMinimumMeters_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_TrackingRangeMaximumMeters_Float: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

int32_t CTrackedDevice::GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *error)
{
//	TRACE(__FUNCTIONW__);
	const int32_t default_value = 0;


	switch (prop) {
	case vr::Prop_EdidVendorID_Int32:
		if (error)
			*error = vr::TrackedProp_Success;
		return 0x22fa;
	case vr::Prop_EdidProductID_Int32:
		if (error)
			*error = vr::TrackedProp_Success;
		return 0x0101;
	case vr::Prop_Axis0Type_Int32: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_Axis1Type_Int32: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_Axis2Type_Int32: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_Axis3Type_Int32: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_Axis4Type_Int32: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint64_t CTrackedDevice::GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *error)
{
//	TRACE(__FUNCTIONW__);
	const uint64_t default_value = 0;

	switch (prop) {
	case vr::Prop_CurrentUniverseId_Uint64: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_PreviousUniverseId_Uint64: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_SupportedButtons_Uint64: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_DisplayFirmwareVersion_Uint64:
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

HmdMatrix34_t CTrackedDevice::GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *error)
{
//	TRACE(__FUNCTIONW__);
	// Default value is identity matrix
	vr::HmdMatrix34_t default_value;
	float s[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1 };
	memcpy(&default_value, s, sizeof(default_value));

	switch (prop) {
	case vr::Prop_StatusDisplayTransform_Matrix34: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint32_t CTrackedDevice::GetStringTrackedDeviceProperty(ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, ETrackedPropertyError *error)
{
//	TRACE(__FUNCTIONW__);
	const uint32_t default_value = 0;

	switch (prop) {
	case vr::Prop_TrackingSystemName_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_ModelNumber_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_SerialNumber_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_RenderModelName_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_ManufacturerName_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_TrackingFirmwareVersion_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_HardwareRevision_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_AttachedDeviceId_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_AllWirelessDongleDescriptions_String:
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_ConnectedWirelessDongle_String:
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

VRControllerState_t CTrackedDevice::GetControllerState()
{
//	TRACE(__FUNCTIONW__);
	vr::VRControllerState_t controller_state = {};

	return controller_state;
}

bool CTrackedDevice::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::HasCamera()
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::GetCameraFirmwareDescription(char * pBuffer, uint32_t nBufferLen)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::GetCameraFrameDimensions(vr::ECameraVideoStreamFormat nVideoStreamFormat, uint32_t * pWidth, uint32_t * pHeight)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::GetCameraFrameBufferingRequirements(int * pDefaultFrameQueueSize, uint32_t * pFrameBufferDataSize)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::SetCameraFrameBuffering(int nFrameBufferCount, void ** ppFrameBuffers, uint32_t nFrameBufferDataSize)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::SetCameraVideoStreamFormat(vr::ECameraVideoStreamFormat nVideoStreamFormat)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

vr::ECameraVideoStreamFormat CTrackedDevice::GetCameraVideoStreamFormat()
{
//	TRACE(__FUNCTIONW__);
	return vr::CVS_FORMAT_UNKNOWN;
}

bool CTrackedDevice::StartVideoStream()
{
//	TRACE(__FUNCTIONW__);
	return false;
}

void CTrackedDevice::StopVideoStream()
{
//	TRACE(__FUNCTIONW__);
}

bool CTrackedDevice::IsVideoStreamActive()
{
//	TRACE(__FUNCTIONW__);
	return false;
}

float CTrackedDevice::GetVideoStreamElapsedTime()
{
//	TRACE(__FUNCTIONW__);
	return 0.0f;
}

const vr::CameraVideoStreamFrame_t * CTrackedDevice::GetVideoStreamFrame()
{
//	TRACE(__FUNCTIONW__);
	return nullptr;
}

void CTrackedDevice::ReleaseVideoStreamFrame(const vr::CameraVideoStreamFrame_t * pFrameImage)
{
//	TRACE(__FUNCTIONW__);
}

bool CTrackedDevice::SetAutoExposure(bool bEnable)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::PauseVideoStream()
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::ResumeVideoStream()
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::IsVideoStreamPaused()
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::GetCameraDistortion(float flInputU, float flInputV, float * pflOutputU, float * pflOutputV)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::GetCameraProjection(float flWidthPixels, float flHeightPixels, float flZNear, float flZFar, vr::HmdMatrix44_t * pProjection)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::GetRecommendedCameraUndistortion(uint32_t * pUndistortionWidthPixels, uint32_t * pUndistortionHeightPixels)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::SetCameraUndistortion(uint32_t nUndistortionWidthPixels, uint32_t nUndistortionHeightPixels)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::GetCameraFirmwareVersion(uint64_t * pFirmwareVersion)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

bool CTrackedDevice::SetFrameRate(int nISPFrameRate, int nSensorFrameRate)
{
//	TRACE(__FUNCTIONW__);
	return false;
}

void CTrackedDevice::CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, void *(*pSharedTextureHandles)[2]) 
{
}

void CTrackedDevice::DestroySwapTextureSet(void *pSharedTextureHandle) 
{
}

void CTrackedDevice::DestroyAllSwapTextureSets(uint32_t unPid) 
{
}

void CTrackedDevice::SubmitLayer(void *pSharedTextureHandles[2], const vr::VRTextureBounds_t * pBounds, const vr::HmdMatrix34_t * pPose) 
{
}

void CTrackedDevice::Present() 
{
}