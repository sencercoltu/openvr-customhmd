#include <process.h>
//#include "Serial.h"
#include "TrackedHMD.h"


CTrackedHMD::CTrackedHMD(std::string id, CServerDriver *pServer) : CTrackedDevice(id, pServer)
{
	IVRSettings *pSettings = m_pDriverHost ? m_pDriverHost->GetSettings(vr::IVRSettings_Version) : nullptr;

	m_hThread = nullptr;
	m_IsRunning = false;
	m_HMDData = { 0 };
	m_HMDData.PIDValue = 0.05f;
	m_HMDData.PosX = 0;
	m_HMDData.PosY = 0;
	m_HMDData.ScreenWidth = 1280;
	m_HMDData.ScreenHeight = 720;
	m_HMDData.AspectRatio = ((float)(m_HMDData.ScreenHeight - 30) / 2.0f) / (float)m_HMDData.ScreenWidth;
	m_HMDData.Frequency = 60;
	m_HMDData.IsConnected = true;
	m_HMDData.FakePackDetected = true;
	m_HMDData.SuperSample = 1.0f;
	m_HMDData.PoseUpdated = false;
	m_HMDData.hPoseLock = CreateMutex(NULL, FALSE, L"PoseLock");

	wcscpy_s(m_HMDData.Port, L"\\\\.\\COM3");
	wcscpy_s(m_HMDData.Model, L"SNYD602");

	m_HMDData.Pose = {};
	m_HMDData.Pose.vecDriverFromHeadTranslation[0] = 0.0;
	m_HMDData.Pose.vecDriverFromHeadTranslation[1] = 0.0;
	m_HMDData.Pose.vecDriverFromHeadTranslation[2] = 0.0;

	//m_Pose.vecPosition[1] = 1.0;

	m_HMDData.Pose.qRotation = HmdQuaternion_Init(1, 0, 0, 0);
	m_HMDData.Pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
	m_HMDData.Pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);


	HMDLog *pLog = new HMDLog(m_pLog);

	if (pSettings)
	{
		char value[128];

		m_HMDData.DirectMode = pSettings->GetBool("steamvr", "directMode", false);

		value[0] = 0;
		pSettings->GetString("driver_customhmd", "monitor", value, sizeof(value), "SNYD602");
		if (value[0])
		{
			std::string basic_string(value);
			std::wstring wchar_value(basic_string.begin(), basic_string.end());
			wcscpy_s(m_HMDData.Model, wchar_value.c_str());

			pLog->Log(m_HMDData.Model);
			pLog->Log(" - HMD: MODEL OK\n");
		}

		value[0] = 0;
		pSettings->GetString("driver_customhmd", "port", value, sizeof(value), "\\\\.\\COM3");
		if (value[0])
		{
			std::string basic_string(value);
			std::wstring wchar_value(basic_string.begin(), basic_string.end());
			wcscpy_s(m_HMDData.Port, wchar_value.c_str());
			pLog->Log(m_HMDData.Port);
			pLog->Log(" - HMD: PORT OK\n");
		}

		m_HMDData.SuperSample = pSettings->GetFloat("driver_customhmd", "supersample", 1.0f);
	}

	m_HMDData.Logger = pLog;
	pLog->Log("HMD: Enumerating monitors...\n");
	if (!m_HMDData.DirectMode)
		EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&m_HMDData);

	pLog->Log("HMD: Done.\n");

}
 
CTrackedHMD::~CTrackedHMD()
{
	//Deactivate();
	delete m_HMDData.Logger;
	CloseHandle(m_HMDData.hPoseLock);
}

unsigned int WINAPI CTrackedHMD::ProcessThread(void *p)
{
	//	TRACE(__FUNCTIONW__);
	//MessageBox(NULL, L"Thread start", L"Info", 0);
	if (!p) return -1;
	auto device = static_cast<CTrackedHMD *>(p);
	if (device)
		device->Run();
	_endthreadex(0);
	return 0;
}

void CTrackedHMD::OpenUSB(hid_device **ppHandle)
{
	CloseUSB(ppHandle);
	hid_device *handle = hid_open(0x104d, 0x1974, nullptr);
	if (!handle)
		return;
	*ppHandle = handle;
	hid_set_nonblocking(handle, 0);
}

void CTrackedHMD::CloseUSB(hid_device **ppHandle)
{
	if (!ppHandle || !*ppHandle)
		return;
	hid_close(*ppHandle);
	*ppHandle = nullptr;
}


void CTrackedHMD::Run()
{
	//return;
	m_pLog->Log("Thread start\n");
	m_pDriverHost->TrackedDevicePropertiesChanged(m_unObjectId);

	//return;

	//	TRACE(__FUNCTIONW__);
	//HTData htData = {};	
	//char *array = (char *)&htData;
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

	//CSerial *pSerial = new CSerial(std::wstring(m_HMDData.Port), 115200);
	//std::string incBuffer;


	int res;

	hid_init();


	//char data;
	float step = 0.0001f;
	auto keydown = false;
	auto lastdown = GetTickCount();
	//auto firstPacket = false;
	DWORD delay = 1000;

	long count = 0;

	//Sleep(5000);
	//if (m_HMDData.IsConnected && m_HMDData.DisplayName[0])
	//{
	//	DEVMODE settings = {};
	//	settings.dmSize = sizeof(DEVMODE);
	//	Sleep(5000);
	//	if (EnumDisplaySettings(m_HMDData.DisplayName, ENUM_CURRENT_SETTINGS, &settings))
	//	{
	//		MessageBox(nullptr, L"xx", L"yy", 0);
	//	}
	//}

	m_HMDData.Pose.poseIsValid = true;
	m_HMDData.Pose.result = ETrackingResult::TrackingResult_Running_OK;
	m_HMDData.Pose.willDriftInYaw = false;
	m_HMDData.Pose.shouldApplyHeadModel = true;

	unsigned char buf[33] = { 0 };
	buf[0] = 0x01;
	buf[1] = 0x81;
	QOrient *pOrient = (QOrient *)buf;
	hid_device *pHandle = nullptr;

	while (m_IsRunning)
	{
		Sleep(1);
		if (delay < 20)
			delay = 20;

		if (keydown && GetTickCount() - lastdown > delay)
			keydown = false;

		if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) && ((GetAsyncKeyState(VK_RCONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				//system button
				keydown = true;
				lastdown = GetTickCount();
				delay /= 2;
				m_pDriverHost->TrackedDeviceButtonPressed(0, k_EButton_System, 0);
				Sleep(1);
				m_pDriverHost->TrackedDeviceButtonUnpressed(0, k_EButton_System, 0);
			}
		}
		else if ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) && ((GetAsyncKeyState(VK_RSHIFT) & 0x8000)))
		{
			if (!keydown)
			{
				//system button
				keydown = true;
				lastdown = GetTickCount();
				delay /= 2;
				m_pDriverHost->TrackedDeviceButtonPressed(0, k_EButton_ApplicationMenu, 0);
				Sleep(1);
				m_pDriverHost->TrackedDeviceButtonUnpressed(0, k_EButton_ApplicationMenu, 0);
			}
		}
		else if ((GetAsyncKeyState(VK_HOME) & 0x8000) && ((GetAsyncKeyState(VK_LCONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				//center
				_yawCenter = _yawRaw;
				_pitchCenter = _pitchRaw;
				_rollCenter = _rollRaw;
				keydown = true;
				lastdown = GetTickCount();
				delay /= 2;
			}
		}
		else if ((GetAsyncKeyState(VK_END) & 0x8000) && ((GetAsyncKeyState(VK_LCONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				//center
				_yawCenter = 0;
				_pitchCenter = 0;
				_rollCenter = 0;
				keydown = true;
				//firstPacket = false;
				lastdown = GetTickCount();
				delay /= 2;
			}
		}
		else if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && ((GetAsyncKeyState(VK_LCONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				m_HMDData.PIDValue += step;
				m_pDriverHost->PhysicalIpdSet(0, m_HMDData.PIDValue);
				keydown = true;
				lastdown = GetTickCount();
				delay /= 2;
			}
		}
		else if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && ((GetAsyncKeyState(VK_LCONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				m_HMDData.PIDValue -= step;
				m_pDriverHost->PhysicalIpdSet(0, m_HMDData.PIDValue);
				keydown = true;
				lastdown = GetTickCount();
				delay /= 2;
			}
		}
		else if ((GetAsyncKeyState(VK_UP) & 0x8000) && ((GetAsyncKeyState(VK_LCONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				m_HMDData.Pose.vecPosition[1] += 0.1;
				keydown = true;
				lastdown = GetTickCount();
				delay /= 2;
			}
		}
		else if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && ((GetAsyncKeyState(VK_LCONTROL) & 0x8000)))
		{
			if (!keydown)
			{
				m_HMDData.Pose.vecPosition[1] -= 0.1;
				keydown = true;
				lastdown = GetTickCount();
				delay /= 2;
			}
		}
		else
		{
			delay = 1000;
			keydown = false;
		}
		if (!pHandle)
		{
			OpenUSB(&pHandle);
			if (!pHandle)
			{
				MessageBox(nullptr, L"No USB", L"Info", 0);
				Sleep(1000);
				continue;
			}
		}
		else
		{			
			res = hid_read_timeout(pHandle, buf, sizeof(buf), 100);
			if (res)
			{
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
				{
					m_HMDData.Pose.qRotation.w = pOrient->w;
					m_HMDData.Pose.qRotation.x = pOrient->x;
					m_HMDData.Pose.qRotation.y = pOrient->y;
					m_HMDData.Pose.qRotation.z = pOrient->z;
					m_HMDData.PoseUpdated = true;
					ReleaseMutex(m_HMDData.hPoseLock);
				}
			}
			else if (res < 0)
			{
				//usb fucked up?				
				MessageBox(nullptr, L"Negative", L"Info", 0);
				CloseUSB(&pHandle);
				Sleep(1000);
				continue;
			}
			else
			{
				MessageBox(nullptr, L"Zero", L"Info", 0);
			}
		}

		//m_pDriverHost->TrackedDevicePoseUpdated(m_unObjectId, m_HMDData.Pose);
	}

	CloseUSB(&pHandle);
	hid_exit();
	m_pLog->Log("Thread stop\n");
}

EVRInitError CTrackedHMD::Activate(uint32_t unObjectId)
{
	m_pLog->Log(__FUNCTION__"\n");
	m_HMDData.Pose.poseIsValid = true;
	m_HMDData.Pose.result = TrackingResult_Running_OK;
	m_HMDData.Pose.deviceIsConnected = true;

	m_HMDData.Pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
	m_HMDData.Pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);


	m_unObjectId = unObjectId;
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ProcessThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hThread);
	}
	return vr::VRInitError_None;
}

void CTrackedHMD::Deactivate()
{
	m_pLog->Log(__FUNCTION__"\n");
	m_IsRunning = false;
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}
	m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
	//	TRACE(__FUNCTIONW__);
}

void *CTrackedHMD::GetComponent(const char *pchComponentNameAndVersion)
{
	m_pLog->Log(__FUNCTION__"\n");
	if (!_stricmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version))
	{
		return (vr::IVRDisplayComponent*)this;
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
	m_HMDData.Logger->Log(__FUNCTION__": %s\n", m_HMDData.DirectMode ? "false" : "true");
	return !m_HMDData.DirectMode;
}

bool CTrackedHMD::IsDisplayRealDisplay()
{
	m_pLog->Log(__FUNCTION__": true\n");
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
	vr::DistortionCoordinates_t coords = {};
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
	vr::DriverPose_t pose;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
	{
		pose = m_HMDData.Pose;
		ReleaseMutex(m_HMDData.hPoseLock);
	}
	else
		return m_HMDData.Pose;
	return pose;
}

bool CTrackedHMD::GetBoolTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	//m_pLog->Log(__FUNCTION__"\n");

	switch (prop)
	{
	case vr::Prop_IsOnDesktop_Bool:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return !m_HMDData.DirectMode;

	case vr::Prop_HasCamera_Bool:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return false;

	case vr::Prop_ContainsProximitySensor_Bool:
		//if (pError)
		//	*pError = vr::TrackedProp_Success;
		//return true;
	case vr::Prop_CanUnifyCoordinateSystemWithHmd_Bool:
	case vr::Prop_DeviceProvidesBatteryStatus_Bool:
	case vr::Prop_WillDriftInYaw_Bool:
	case vr::Prop_BlockServerShutdown_Bool:
	case vr::Prop_DeviceIsWireless_Bool:
	case vr::Prop_DeviceIsCharging_Bool:
	case vr::Prop_Firmware_UpdateAvailable_Bool:
	case vr::Prop_Firmware_ManualUpdate_Bool:
	case vr::Prop_DeviceCanPowerOff_Bool:
	case vr::Prop_ReportsTimeSinceVSync_Bool:
		if (pError)
			*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return false;
	}

	if (pError)
		*pError = vr::TrackedProp_UnknownProperty;
	return false;
}

float CTrackedHMD::GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError)
{
	//m_pLog->Log(__FUNCTION__"\n");
	const float default_value = 0.0f;

	switch (prop)
	{
	case vr::Prop_UserHeadToEyeDepthMeters_Float:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return 0.0f;
	case vr::Prop_SecondsFromVsyncToPhotons_Float:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return 0.0f;
	case vr::Prop_DisplayFrequency_Float:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return m_HMDData.Frequency;
	case vr::Prop_UserIpdMeters_Float:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return m_HMDData.PIDValue;
	case vr::Prop_DeviceBatteryPercentage_Float:
	case vr::Prop_DisplayGCScale_Float:
	case vr::Prop_DisplayGCPrescale_Float:
	case vr::Prop_DisplayMCOffset_Float:
	case vr::Prop_DisplayMCScale_Float:
	case vr::Prop_DisplayGCBlackClamp_Float:
	case vr::Prop_DisplayGCOffset_Float:
	case vr::Prop_LensCenterLeftU_Float:
	case vr::Prop_LensCenterLeftV_Float:
	case vr::Prop_LensCenterRightU_Float:
	case vr::Prop_LensCenterRightV_Float:
	case vr::Prop_FieldOfViewLeftDegrees_Float:
	case vr::Prop_FieldOfViewRightDegrees_Float:
	case vr::Prop_FieldOfViewTopDegrees_Float:
	case vr::Prop_FieldOfViewBottomDegrees_Float:
	case vr::Prop_TrackingRangeMinimumMeters_Float:
	case vr::Prop_TrackingRangeMaximumMeters_Float:
		if (pError)
			*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

	if (pError)
		*pError = vr::TrackedProp_UnknownProperty;
	return default_value;
}

int32_t CTrackedHMD::GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	//m_pLog->Log(__FUNCTION__"\n");
	const int32_t default_value = 0;

	switch (prop)
	{
	case vr::Prop_DeviceClass_Int32:
		m_pLog->Log("Prop_DeviceClass_Int32\n");
		if (pError)
			*pError = vr::TrackedProp_Success;
		return vr::TrackedDeviceClass_HMD;
	case vr::Prop_EdidVendorID_Int32:
		m_pLog->Log("Prop_EdidVendorID_Int32\n");
		if (pError)
			*pError = vr::TrackedProp_Success;
		return 0x094D; // 0x4D09; //0x1002; // 
	case vr::Prop_EdidProductID_Int32:
		m_pLog->Log("Prop_EdidProductID_Int32\n");
		if (pError)
			*pError = vr::TrackedProp_Success;
		return 0xD602; // 0x02D6; //0x6613; //
	//case vr::Prop_DisplayMCType_Int32:
	//case vr::Prop_DisplayGCType_Int32:
	//case vr::Prop_Axis0Type_Int32:
	//case vr::Prop_Axis1Type_Int32:
	//case vr::Prop_Axis2Type_Int32:
	//case vr::Prop_Axis3Type_Int32:
	//case vr::Prop_Axis4Type_Int32:
	//	if (pError)
	//		*pError = vr::TrackedProp_ValueNotProvidedByDevice;
	//	return default_value;
	}

	if (pError)
		*pError = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint64_t CTrackedHMD::GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
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

HmdMatrix34_t CTrackedHMD::GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError)
{
	//m_pLog->Log(__FUNCTION__"\n");
	// Default value is identity matrix
	vr::HmdMatrix34_t default_value;
	HmdMatrix_SetIdentity(&default_value);

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

uint32_t CTrackedHMD::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError *pError)
{
	//m_pLog->Log(__FUNCTION__"\n");
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

std::string CTrackedHMD::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError)
{
	//m_pLog->Log(__FUNCTION__"\n");
	*pError = vr::TrackedProp_ValueNotProvidedByDevice;
	std::string sRetVal;

	switch (prop)
	{
	case vr::Prop_ManufacturerName_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return std::string("SONY");
	case vr::Prop_ModelNumber_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return std::string("HMZ-T2");
	case vr::Prop_SerialNumber_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return std::string("1244244");
	case vr::Prop_TrackingSystemName_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return std::string("Custom Aruino Tracker");
	case vr::Prop_HardwareRevision_String:
	case vr::Prop_TrackingFirmwareVersion_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return std::string("1.0");
	case vr::Prop_AttachedDeviceId_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return m_Id;
	case vr::Prop_CameraFirmwareDescription_String:
		if (pError)
			*pError = vr::TrackedProp_Success;
		return std::string("Sony HMZ-T2 Camera");

	case vr::Prop_RenderModelName_String:
	case vr::Prop_AllWirelessDongleDescriptions_String:
	case vr::Prop_ConnectedWirelessDongle_String:
	case vr::Prop_Firmware_ManualUpdateURL_String:
	case vr::Prop_Firmware_ProgrammingTarget_String:
	case vr::Prop_DisplayMCImageLeft_String:
	case vr::Prop_DisplayMCImageRight_String:
	case vr::Prop_DisplayGCImage_String:
	case vr::Prop_ModeLabel_String:
	default:
		if (pError)
			*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return sRetVal;
	}
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

void CTrackedHMD::SubmitLayer(void *pSharedTextureHandles[2], const vr::VRTextureBounds_t(&bounds)[2], const vr::HmdMatrix34_t *pPose)
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

void CTrackedHMD::RunFrame()
{
	vr::DriverPose_t pose;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
	{
		if (m_HMDData.PoseUpdated)
		{
			pose = m_HMDData.Pose;
			m_HMDData.PoseUpdated = false;
		}
		ReleaseMutex(m_HMDData.hPoseLock);
	}
	else
		return;

	m_pDriverHost->TrackedDevicePoseUpdated(m_unObjectId, pose);
}