#include <process.h>
#include "TrackedHMD.h"
#include <mfapi.h>
#include "UdpClientServer.h"


CTrackedHMD::CTrackedHMD(std::string displayName, CServerDriver *pServer) : CTrackedDevice(displayName, pServer)
{
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	vr::EVRSettingsError error;
	//	m_hTJ = tjInitCompress();

	ZeroMemory(&m_DirectScreen, sizeof(m_DirectScreen));

	m_FrameCount = 0;
	//m_pScreenImage = nullptr;
	//m_pImage = nullptr;
	//m_pPrevImage = nullptr;
	//m_pDiffImage = nullptr;

	m_SyncTexture = 0;
	m_pSyncTexture = nullptr;


	m_hBufferLock = CreateMutex(nullptr, FALSE, L"BufferLock");
	m_hTextureMapLock = CreateMutex(nullptr, FALSE, L"TextureMapLock");
	//m_CompressedBufferSize = 0;
	m_SyncTexture = 0;
	//	m_PixelBufferSize = 0;
	//	m_pPixelBuffer = nullptr;
	m_pContext = nullptr;
	//m_pScreenTexture = nullptr;
	//m_pScreenResource = nullptr;
	m_pDevice = nullptr;
	m_FeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;

	NamedIconPathDeviceOff = "{customhmd}headset_status_off.png";
	NamedIconPathDeviceSearching = "{customhmd}headset_status_searching.gif";
	NamedIconPathDeviceSearchingAlert = "{customhmd}headset_status_searching_alert.gif";
	NamedIconPathDeviceReady = "{customhmd}headset_status_ready.png";
	NamedIconPathDeviceReadyAlert = "{customhmd}headset_status_ready_alert.png";
	NamedIconPathDeviceNotReady = "{customhmd}headset_status_error.png";
	NamedIconPathDeviceStandby = "{customhmd}headset_status_standby.png";
	NamedIconPathDeviceAlertLow = "{customhmd}headset_status_error.png";

	TrackingSystemName = "Custom HMD";
	ModelNumber = "3CPO";
	SerialNumber = "HMD-1244244";
	RenderModelName = "generic_hmd";
	ManufacturerName = "DIY";
	AllWirelessDongleDescriptions = "HMD-None";
	ConnectedWirelessDongle = "HMD-None";
	Firmware_ProgrammingTarget = "HMD-Multi";
	FirmwareVersion = 1462663157;
	HardwareRevision = 2147614976;
	FPGAVersion = 262;
	DongleVersion = 1461100729;
	ContainsProximitySensor = false;
	DeviceClass = TrackedDeviceClass_HMD;
	HasCamera = true;

	ReportsTimeSinceVSync = false;
	SecondsFromVsyncToPhotons = 0.0f;
	DisplayFrequency = 60.0f;
	UserIpdMeters = 0.05f;
	CurrentUniverseId = 2;
	PreviousUniverseId = 0;
	DisplayFirmwareVersion = 2097504;
	IsOnDesktop = false;
	DisplayMCType = 0;
	DisplayMCOffset = 0.0f;
	DisplayMCScale = 0.0f;
	DisplayMCImageLeft = "";
	DisplayMCImageRight = "";
	DisplayGCBlackClamp = 0.0f;
	EdidVendorID = m_pSettings->GetInt32("driver_customhmd", "edid_vid", &error);
	if (error != VRSettingsError_None) EdidVendorID = 0xD94D;
	EdidProductID = m_pSettings->GetInt32("driver_customhmd", "edid_pid", &error);
	if (error != VRSettingsError_None) EdidProductID = 0xD602;
	CameraToHeadTransform = HmdMatrix34_t();
	Quaternion::HmdMatrix_SetIdentity(&CameraToHeadTransform);
	DisplayGCType = 0;
	DisplayGCOffset = 0.0f;
	DisplayGCScale = 0.0f;
	DisplayGCPrescale = 0.0f;
	DisplayGCImage = "";
	LensCenterLeftU = 0.0f;
	LensCenterLeftV = 0.0f;
	LensCenterRightU = 0.0f;
	LensCenterRightV = 0.0f;
	UserHeadToEyeDepthMeters = 0.0f;
	CameraFirmwareVersion = 8590262295;
	CameraFirmwareDescription = "Version: 02.05.0D Date: 2016.Jul.31 Type: HeadSet USB Camera";
	DisplayFPGAVersion = 57;
	DisplayBootloaderVersion = 1048584;
	DisplayHardwareVersion = 19;
	AudioFirmwareVersion = 3;
	CameraCompatibilityMode = CAMERA_COMPAT_MODE_ISO_30FPS;
	ScreenshotHorizontalFieldOfViewDegrees = 0.0f;
	ScreenshotVerticalFieldOfViewDegrees = 0.0f;
	DisplaySuppressed = false;
	DisplayAllowNightMode = true;
	DisplayMCImageWidth = 0;
	DisplayMCImageHeight = 0;
	DisplayMCImageNumChannels = 0;
	DisplayMCImageData = nullptr;
	UsesDriverDirectMode = false;

	ZeroMemory(&m_Camera, sizeof(m_Camera));
	m_Camera.hLock = CreateMutex(nullptr, FALSE, L"CameraLock");

	ZeroMemory(&m_HMDData, sizeof(m_HMDData));
	m_HMDData.pHMDDriver = this;
	m_HMDData.Windowed = false;
	//m_HMDData.IPDValue = 0.05f;
	m_HMDData.PosX = 0;
	m_HMDData.PosY = 0;
	m_HMDData.ScreenWidth = 1280;
	m_HMDData.ScreenHeight = 1470;
	m_HMDData.AspectRatio = ((float)(m_HMDData.ScreenHeight - 30) / 2.0f) / (float)m_HMDData.ScreenWidth;
	m_HMDData.Frequency = 60;
	m_HMDData.IsConnected = true;
	m_HMDData.FakePackDetected = true;
	m_HMDData.SuperSample = 1.0f;
	m_HMDData.PoseUpdated = false;
	m_HMDData.hPoseLock = CreateMutex(NULL, FALSE, L"PoseLock");

	//wcscpy_s(m_HMDData.Port, L"\\\\.\\COM3");
	wcscpy_s(m_HMDData.Model, L"");

	m_HMDData.Pose.willDriftInYaw = false;
	m_HMDData.Pose.shouldApplyHeadModel = false;
	m_HMDData.Pose.deviceIsConnected = true;
	m_HMDData.Pose.poseIsValid = true;
	m_HMDData.Pose.result = ETrackingResult::TrackingResult_Running_OK;
	m_HMDData.Pose.qRotation = Quaternion(1, 0, 0, 0);
	m_HMDData.Pose.qWorldFromDriverRotation = Quaternion(1, 0, 0, 0);
	m_HMDData.Pose.qDriverFromHeadRotation = Quaternion(1, 0, 0, 0);
	//m_HMDData.Pose.vecWorldFromDriverTranslation[2] = -2;
	m_HMDData.Pose.vecDriverFromHeadTranslation[2] = -0.15;
	m_HMDData.Pose.poseTimeOffset = -0.032f;

	if (m_pSettings)
	{
		char value[256] = {};
		m_pSettings->GetString("driver_customhmd", "directStream", value, sizeof(value));
		if (value[0])
		{
			strcpy_s(m_HMDData.DirectStreamURL, value);
			m_HMDData.ScreenWidth = 1920;
			m_HMDData.ScreenHeight = 1080;
			m_HMDData.IsConnected = UsesDriverDirectMode = m_HMDData.DirectMode = true;
			m_HMDData.FakePackDetected = false;
			m_HMDData.Frequency = 60;
			DriverLog("Using remote display %s...", value);

			value[0] = 0;
			m_pSettings->GetString("driver_customhmd", "directResolution", value, sizeof(value));
			if (value[0])
			{
				auto pos = strchr(value, 'x');
				if (pos) 
				{
					*pos = 0;
					float fps;
					int width = atoi(value);
					pos++;
					char *h = pos;
					pos = strchr(value, '@');
					if (pos)
					{
						pos[0] = 0;
						pos++;
						fps = (float) atof(pos);
						if (fps)
							m_HMDData.Frequency = fps;
					}
					int height = atoi(h);
					if (width && height)
					{
						m_HMDData.ScreenWidth = width;
						m_HMDData.ScreenHeight = height;
					}
				}
			}
		}
		else
		{			
			bool steamVrDirectMode = m_pSettings->GetBool("steamvr", "directMode", &error);
			if (error != VRSettingsError_None) steamVrDirectMode = false;
			//we cannot put monitor into directmode (no API for mortals like us :( )
			//so we cannot use direct mode if it's not enabled in steamvr settings. maybe set m_HMDData.DirectMode directly from steamvr's settings...
			m_HMDData.DirectMode = m_pSettings->GetBool("driver_customhmd", "directMode", &error);
			if (error != VRSettingsError_None) m_HMDData.DirectMode = false;

			if (!m_HMDData.DirectMode)
			{
				value[0] = 0;
				m_pSettings->GetString("driver_customhmd", "monitor", value, sizeof(value));
				if (value[0])
				{
					std::string basic_string(value);
					std::wstring wchar_value(basic_string.begin(), basic_string.end());
					wcscpy_s(m_HMDData.Model, wchar_value.c_str());
					DriverLog("Using model %S for detection...", m_HMDData.Model);
				}
			}
		}
		m_HMDData.SuperSample = m_pSettings->GetFloat("driver_customhmd", "supersample");
	}

	//m_HMDData.Logger = m_pLog;

	if (!m_HMDData.DirectMode)
	{
		DriverLog("HMD: Enumerating monitors...");
		EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&m_HMDData);
		DriverLog("HMD: Monitor detection finished.");
		//if any value is set tread as windowed
		error = VRSettingsError_None; int x = m_pSettings->GetInt32("driver_customhmd", "windowX", &error);
		if (error == VRSettingsError_None) { m_HMDData.PosX = x; m_HMDData.Windowed = true; }
		error = VRSettingsError_None; int y = m_pSettings->GetInt32("driver_customhmd", "windowY", &error);
		if (error == VRSettingsError_None) { m_HMDData.PosY = x; m_HMDData.Windowed = true; }
		error = VRSettingsError_None; int w = m_pSettings->GetInt32("driver_customhmd", "windowW", &error);
		if (error == VRSettingsError_None) { m_HMDData.ScreenWidth = w; m_HMDData.Windowed = true; }
		error = VRSettingsError_None; int h = m_pSettings->GetInt32("driver_customhmd", "windowH", &error);
		if (error == VRSettingsError_None) { m_HMDData.ScreenHeight = h; m_HMDData.Windowed = true; }
	}
	else if (!m_HMDData.DirectStreamURL[0])
	{
		m_HMDData.IsConnected = IsD2DConnected(EdidVendorID);
	}

	char desiredCamera[128] = { 0 };
	m_pSettings->GetString("driver_customhmd", "camera", desiredCamera, sizeof(desiredCamera));
	m_Camera.Options.Name = desiredCamera;
	m_Camera.Options.Width = 320;
	m_Camera.Options.Height = 240;
	m_Camera.Options.MediaFormat = MFVideoFormat_NV12;
	m_Camera.StreamFormat = CVS_FORMAT_NV12; //default
	m_Camera.Options.pfCallback = CameraFrameUpdateCallback;
	m_Camera.Options.pUserData = this;

	DisplayFrequency = m_HMDData.Frequency;

	m_HMDData.EyeWidth = m_HMDData.ScreenWidth / 2;

	//one time setup to determine buffersize
	m_Camera.Options.Setup();

	m_hDisplayThread = nullptr;
	m_IsRunning = false;

	if (IsConnected())
	{
		m_pDriverHost->TrackedDeviceAdded(SerialNumber.c_str(), vr::TrackedDeviceClass_HMD, this);
		if (m_HMDData.DirectStreamURL[0]) //starty remote display thread
		{
			m_hDisplayThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, RemoteDisplayThread, this, CREATE_SUSPENDED, nullptr));
			if (m_hDisplayThread)
			{
				m_IsRunning = true;
				ResumeThread(m_hDisplayThread);
			}
			m_hControlThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, RemoteControlThread, this, CREATE_SUSPENDED, nullptr));
			if (m_hControlThread)
			{
				m_IsRunning = true;
				ResumeThread(m_hControlThread);
			}
		}
	}
}

bool CTrackedHMD::IsConnected()
{
	CShMem mem;
	if (((mem.GetState() != Disconnected) || m_HMDData.DirectStreamURL[0]) && m_HMDData.IsConnected)
		return true;
	return false;
}

CTrackedHMD::~CTrackedHMD()
{
	m_IsRunning = false;

	SAFE_THREADCLOSE(m_hControlThread);
	SAFE_THREADCLOSE(m_hDisplayThread);

	SAFE_CLOSE(m_HMDData.hPoseLock);
	SAFE_RELEASE(m_pSyncTexture);
	SAFE_RELEASE(m_pContext);
	SAFE_RELEASE(m_pDevice);
	SAFE_CLOSE(m_hTextureMapLock);
	SAFE_CLOSE(m_hBufferLock);
	CoUninitialize();
}

unsigned int WINAPI CTrackedHMD::RemoteDisplayThread(void *p)
{	
	auto trackedHMD = static_cast<CTrackedHMD *>(p);
	if (trackedHMD)
		trackedHMD->RunRemoteDisplay();
	_endthreadex(0);
	return 0;
}

const uint8_t nalu1[] = { 0, 0, 0, 1};
const uint8_t nalu2[] = { 0, 0, 1 };

void CTrackedHMD::RunRemoteDisplay()
{	
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	UdpClientServer::CUdpClient *pUdpClient = new UdpClientServer::CUdpClient(m_HMDData.DirectStreamURL, 1974);

	m_DirectScreen.Init(m_HMDData.ScreenWidth, m_HMDData.ScreenHeight, m_HMDData.Frequency, m_HMDData.DirectStreamURL);

	InfoPacket infoPacket = {};
	infoPacket.H = 'H'; 
	infoPacket.M = 'M';
	infoPacket.D1 = 'D';
	infoPacket.D2 = 'D';
	infoPacket.Width = m_HMDData.ScreenWidth;
	infoPacket.Height = m_HMDData.ScreenHeight;

	FILE *fp = nullptr;
	//fopen_s(&fp, "D:\\test.h264", "wb");

	while (m_IsRunning)
	{
		if (m_DirectScreen.pPixelBuffer[0])
		{
			if (GetTickCount() - m_DirectScreen.LastPacketReceive > 1000)
			{
				//send reset
				pUdpClient->send((const char*)&infoPacket, sizeof(infoPacket));
				Sleep(100);


				pUdpClient->send((const char*)m_DirectScreen.pCodecContext->extradata, m_DirectScreen.pCodecContext->extradata_size);
				if (fp) fwrite((const char*)m_DirectScreen.pCodecContext->extradata, 1, m_DirectScreen.pCodecContext->extradata_size, fp);
				continue;
			}			

			auto pitch = 0;
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTextureMapLock, 10))
			{
				pitch += UpdateBuffer(&m_DirectScreen.Left);
				pitch += UpdateBuffer(&m_DirectScreen.Right);
				ReleaseMutex(m_hTextureMapLock);
			}

			if (!pitch ||
				m_DirectScreen.Left.Desc.Width != m_DirectScreen.Right.Desc.Width || //left and right res should be same
				m_DirectScreen.Left.Desc.Height != m_DirectScreen.Right.Desc.Height || 
				m_DirectScreen.Left.Desc.Format != m_DirectScreen.Right.Desc.Format)
			{
				Sleep(1); 
				continue;
			}			
			
			if (!m_DirectScreen.ProcessFrame(pitch))
			{
				while (auto pPacket = m_DirectScreen.GetPacket())
				{
					auto remSize = pPacket->size;
					auto *pData = pPacket->data;
					pUdpClient->send((const char*)pData, remSize);
					if (fp) fwrite((const char*)pData, 1, remSize, fp);

					//while (remSize)
					//{
					//	auto pNalu1 = memmem(pData + 3, remSize, nalu1, sizeof(nalu1));
					//	auto pNalu2 = memmem(pData + 3, remSize, nalu2, sizeof(nalu2));
					//	auto *pNext = (uint8_t *) min(pNalu1, pNalu2);
					//	if (!pNext) 
					//	{
					//		//send full buffer
					//		pUdpClient->send((const char*)pData, remSize);
					//		if (fp) fwrite((const char*)pData, 1, remSize, fp);
					//		break;
					//	}
					//	else
					//	{
					//		int size = (int)(pNext - pData);
					//		pUdpClient->send((const char*)pData, size);
					//		if (fp) fwrite((const char*)pData, 1, size, fp);
					//		remSize -= size;
					//		pData += size;
					//	}
					//}
				}
			}

			Sleep(1);
		}
		else
			Sleep(100);
	}
	 
	m_DirectScreen.Destroy();

	if (fp) fclose(fp);
	fp = nullptr;

	delete pUdpClient;
	pUdpClient = nullptr;
	WSACleanup();
}

unsigned int WINAPI CTrackedHMD::RemoteControlThread(void *p)
{
	auto trackedHMD = static_cast<CTrackedHMD *>(p);
	if (trackedHMD)
		trackedHMD->RunRemoteControl();
	_endthreadex(0);
	return 0;
}

void CTrackedHMD::RunRemoteControl()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	USBPacket tUsbPacket = {};

	UdpClientServer::CUdpServer *pUdpServer = new UdpClientServer::CUdpServer("0.0.0.0", 1974);

	while (m_IsRunning)
	{
		if (pUdpServer->timed_recv((char *)&tUsbPacket, sizeof(tUsbPacket), 100) == sizeof(tUsbPacket))
		{
			SetPacketCrc(&tUsbPacket);
			ProcessRemotePacket(&tUsbPacket);
			m_DirectScreen.LastPacketReceive = GetTickCount();
		}
	}

	delete pUdpServer;
	pUdpServer = nullptr;
	WSACleanup();
}


int CTrackedHMD::UpdateBuffer(DirectEyeData *pEyeData)
{
	if (!pEyeData) 
		return false;
	if (!pEyeData->pData) 
		return false;

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	int width = 0, height = 0, stride = 0;
	int fmt = 0;

	if (SUCCEEDED(m_pContext->Map(pEyeData->pData->pCPUResource, 0, D3D11_MAP_READ, 0, &mappedResource)))
	{		
		pEyeData->pData->pCPUTexture->GetDesc(&pEyeData->Desc);
		width = pEyeData->Desc.Width;
		height = pEyeData->Desc.Height;
		stride = mappedResource.RowPitch;		

		//copy eye to pixel buffer. always assume 32 bit pixel				
		auto srcPos = (unsigned char *) mappedResource.pData;
		auto dstPos = m_DirectScreen.pPixelBuffer[0] + (pEyeData->Eye == EVREye::Eye_Right ? stride : 0);

		//copy left eye to left, right eye to right
		for (unsigned int y = 0; y < pEyeData->Desc.Height; y++)
		{
			memcpy(dstPos, srcPos, stride);
			dstPos += stride;
			srcPos += stride;
		}
		m_pContext->Unmap(pEyeData->pData->pCPUResource, 0);
	}
	pEyeData->pData = nullptr;
	return stride;
}


void CTrackedHMD::ProcessRemotePacket(USBPacket *pPacket)
{
	m_pServer->ProcessUSBPacket(pPacket);
}

EVRInitError CTrackedHMD::Activate(uint32_t unObjectId)
{
	DriverLog(__FUNCTION__" idx: %d", unObjectId);
	m_unObjectId = unObjectId;
	SetDefaultProperties();
	return VRInitError_None;
}

void CTrackedHMD::SetDefaultProperties()
{
	CTrackedDevice::SetDefaultProperties();
	ETrackedPropertyError error;


	error = SET_PROP(String, NamedIconPathDeviceOff, .c_str());
	error = SET_PROP(String, NamedIconPathDeviceSearching, .c_str());
	error = SET_PROP(String, NamedIconPathDeviceSearchingAlert, .c_str());
	error = SET_PROP(String, NamedIconPathDeviceReady, .c_str());
	error = SET_PROP(String, NamedIconPathDeviceReadyAlert, .c_str());
	error = SET_PROP(String, NamedIconPathDeviceNotReady, .c_str());
	error = SET_PROP(String, NamedIconPathDeviceStandby, .c_str());
	error = SET_PROP(String, NamedIconPathDeviceAlertLow, .c_str());

	error = SET_PROP(Bool, ReportsTimeSinceVSync, );
	error = SET_PROP(Bool, IsOnDesktop, );
	error = SET_PROP(Bool, DisplaySuppressed, );
	error = SET_PROP(Bool, DisplayAllowNightMode, );
	error = SET_PROP(Bool, UsesDriverDirectMode, );

	error = SET_PROP(Float, SecondsFromVsyncToPhotons, );
	error = SET_PROP(Float, DisplayFrequency, );
	error = SET_PROP(Float, UserIpdMeters, );
	error = SET_PROP(Float, DisplayMCOffset, );
	error = SET_PROP(Float, DisplayMCScale, );
	error = SET_PROP(Float, DisplayGCBlackClamp, );
	error = SET_PROP(Float, DisplayGCOffset, );
	error = SET_PROP(Float, DisplayGCScale, );
	error = SET_PROP(Float, DisplayGCPrescale, );
	error = SET_PROP(Float, LensCenterLeftU, );
	error = SET_PROP(Float, LensCenterLeftV, );
	error = SET_PROP(Float, LensCenterRightU, );
	error = SET_PROP(Float, LensCenterRightV, );
	error = SET_PROP(Float, UserHeadToEyeDepthMeters, );
	error = SET_PROP(Float, ScreenshotHorizontalFieldOfViewDegrees, );
	error = SET_PROP(Float, ScreenshotVerticalFieldOfViewDegrees, );

	error = SET_PROP(Uint64, CurrentUniverseId, );
	error = SET_PROP(Uint64, PreviousUniverseId, );
	error = SET_PROP(Uint64, DisplayFirmwareVersion, );
	error = SET_PROP(Uint64, CameraFirmwareVersion, );
	error = SET_PROP(Uint64, DisplayFPGAVersion, );
	error = SET_PROP(Uint64, DisplayBootloaderVersion, );
	error = SET_PROP(Uint64, DisplayHardwareVersion, );
	error = SET_PROP(Uint64, AudioFirmwareVersion, );

	error = SET_PROP(Int32, DisplayMCType, );
	error = SET_PROP(Int32, EdidVendorID, );
	error = SET_PROP(Int32, EdidProductID, );
	error = SET_PROP(Int32, DisplayGCType, );
	error = SET_PROP(Int32, CameraCompatibilityMode, );
	error = SET_PROP(Int32, DisplayMCImageWidth, );
	error = SET_PROP(Int32, DisplayMCImageHeight, );
	error = SET_PROP(Int32, DisplayMCImageNumChannels, );

	error = SET_PROP(String, DisplayMCImageLeft, .c_str());
	error = SET_PROP(String, DisplayMCImageRight, .c_str());
	error = SET_PROP(String, DisplayGCImage, .c_str());
	error = SET_PROP(String, CameraFirmwareDescription, .c_str());

	error = m_pProperties->SetProperty(m_ulPropertyContainer, Prop_StatusDisplayTransform_Matrix34, &CameraToHeadTransform, sizeof(CameraToHeadTransform), k_unHmdMatrix34PropertyTag);

	//?? void *DisplayMCImageData;
}

void CTrackedHMD::Deactivate()
{
	DriverLog(__FUNCTION__);
	m_Camera.Destroy();
	m_unObjectId = k_unTrackedDeviceIndexInvalid;
	//	TRACE(__FUNCTIONW__);
}

void CTrackedHMD::EnterStandby()
{

}

void *CTrackedHMD::GetComponent(const char *pchComponentNameAndVersion)
{
	DriverLog(__FUNCTION__" %s", pchComponentNameAndVersion);

	if (!_stricmp(pchComponentNameAndVersion, IVRDriverDirectModeComponent_Version))
	{
		return (IVRDriverDirectModeComponent*)this;
	}

	if (!_stricmp(pchComponentNameAndVersion, IVRDisplayComponent_Version))
	{
		return (IVRDisplayComponent*)this;
	}

	if (!_stricmp(pchComponentNameAndVersion, IVRCameraComponent_Version))
	{
		return (IVRCameraComponent*)this;
	}

	if (!_stricmp(pchComponentNameAndVersion, ITrackedDeviceServerDriver_Version))
	{
		return (ITrackedDeviceServerDriver*)this;
	}

	return nullptr;
}

void CTrackedHMD::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
	DriverLog(__FUNCTION__" %s", pchRequest);
}

void CTrackedHMD::GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
	*pnX = m_HMDData.PosX;
	*pnY = m_HMDData.PosY;
	*pnWidth = m_HMDData.ScreenWidth;
	*pnHeight = m_HMDData.ScreenHeight;
	DriverLog(__FUNCTION__" x: %d, y: %d, w: %d, h: %d", *pnX, *pnY, *pnWidth, *pnHeight);
}

bool CTrackedHMD::IsDisplayOnDesktop()
{
	DriverLog(__FUNCTION__" returning %d", !m_HMDData.DirectMode);
	return !m_HMDData.DirectMode;
}

bool CTrackedHMD::IsDisplayRealDisplay()
{
	DriverLog(__FUNCTION__" returning %d", !m_HMDData.Windowed && !m_HMDData.DirectStreamURL[0]);
	return !m_HMDData.Windowed && !m_HMDData.DirectStreamURL[0];
}

void CTrackedHMD::GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight)
{
	*pnWidth = uint32_t((m_HMDData.DirectStreamURL[0] ? m_HMDData.EyeWidth : m_HMDData.ScreenWidth) * m_HMDData.SuperSample);
	*pnHeight = uint32_t((m_HMDData.FakePackDetected ? (m_HMDData.ScreenHeight - 30) / 2 : m_HMDData.ScreenHeight) * m_HMDData.SuperSample);
	DriverLog(__FUNCTION__" w: %d, h: %d", *pnWidth, *pnHeight);
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
			*pnWidth = m_HMDData.EyeWidth;
			*pnHeight = m_HMDData.ScreenHeight;
			break;
		case EVREye::Eye_Right:
			*pnX = 0 + m_HMDData.EyeWidth;
			*pnY = 0;
			*pnWidth = m_HMDData.EyeWidth;
			*pnHeight = m_HMDData.ScreenHeight;
			break;
		}
	}
	DriverLog(__FUNCTION__" Eye: %d, x: %d, y: %d, w: %d, h: %d", eEye, *pnX, *pnY, *pnWidth, *pnHeight);
}

void CTrackedHMD::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom)
{
	auto k = m_HMDData.FakePackDetected ? m_HMDData.AspectRatio : (1.0f / m_HMDData.AspectRatio);
	switch (eEye)
	{
	case EVREye::Eye_Left:
		*pfLeft = -1.0f;
		*pfRight = 1.0f;
		*pfTop = -1.0f * k;
		*pfBottom = 1.0f * k;
		break;
	case EVREye::Eye_Right:
		*pfLeft = -1.0f;
		*pfRight = 1.0f;
		*pfTop = -1.0f * k;
		*pfBottom = 1.0f * k;
		break;
	}
	DriverLog(__FUNCTION__" Eye: %d, l: %f, r: %f, t: %f, b: %f", eEye, *pfLeft, *pfRight, *pfTop, *pfBottom);
}

DistortionCoordinates_t CTrackedHMD::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	//DriverLog(__FUNCTION__" Eye: %d, fU: %f, fV: %f", eEye, fU, fV);
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

void CTrackedHMD::CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3])
{
	DriverLog(__FUNCTION__" Create TexSwapSet %u: fmt(%u) size(%ux%u)", unPid, unFormat, unWidth, unHeight);

	if (!m_pDevice)
	{
		D3D_FEATURE_LEVEL levels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1
		};

		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
			levels,
			ARRAYSIZE(levels),
			D3D11_SDK_VERSION,
			&m_pDevice,
			&m_FeatureLevel,
			&m_pContext);

		//D3D11_TEXTURE2D_DESC Desc = {};
		//Desc.ArraySize = 1;
		//Desc.Width = unWidth * 2; //first texture is left eye? alloc for both eyes
		//Desc.Height = unHeight;
		//Desc.MipLevels = 1;
		//Desc.SampleDesc.Count = 1;
		//Desc.SampleDesc.Quality = 0;
		//Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		//Desc.Usage = D3D11_USAGE_STAGING;
		//Desc.BindFlags = 0;
		//Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		//Desc.MiscFlags = 0;

		//m_HMDData.EyeTexWidth = unWidth;
		//m_HMDData.EyeTexHeight = unHeight;

	}

	if (!m_pDevice)
		return;

	auto set = new TextureSet;
	ZeroMemory(set, sizeof(TextureSet));
	set->Pid = unPid;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.ArraySize = 1;
	desc.Width = unWidth;
	desc.Height = unHeight;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Format = (DXGI_FORMAT)unFormat;

	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTextureMapLock, 10000))
	{
		for (auto i = 0; i < 3; i++)
		{
			//create GPU texture

			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;

			HRESULT hr = m_pDevice->CreateTexture2D(&desc, nullptr, &(set->Data[i].pGPUTexture));
			if (hr == S_OK)
			{
				set->Data[i].Index = i;
				set->Data[i].pGPUTexture->QueryInterface(__uuidof(ID3D11Resource), (void**)&set->Data[i].pGPUResource);

				IDXGIResource1 *pDXIResource1 = nullptr;
				if (SUCCEEDED(set->Data[i].pGPUTexture->QueryInterface(__uuidof(IDXGIResource1), (void**)&pDXIResource1)))
				{
					hr = pDXIResource1->GetSharedHandle(&set->Data[i].hSharedHandle);
					pDXIResource1->Release();
					(*pSharedTextureHandles)[i] = (SharedTextureHandle_t)set->Data[i].hSharedHandle;
				}

				//set->Data[i].pScratchImage = new DirectX::ScratchImage();
				//set->Data[i].pScratchImage->Initialize2D(desc.Format, unWidth, unHeight, 1, 1);
				//set->Data[i].pImage = set->Data[i].pScratchImage->GetImage(0, 0, 0);



				TextureLink tl = {};
				tl.pData = &set->Data[i];
				tl.pSet = set;
				m_TextureMap[(SharedTextureHandle_t)set->Data[i].hSharedHandle] = tl;
			}

			//create CPU texture

			//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			if (SUCCEEDED(m_pDevice->CreateTexture2D(&desc, nullptr, &(set->Data[i].pCPUTexture))))
				set->Data[i].pCPUTexture->QueryInterface(__uuidof(ID3D11Resource), (void**)&set->Data[i].pCPUResource);
		}

		m_TextureSets.push_back(set);

		ReleaseMutex(m_hTextureMapLock);
	}
}

void CTrackedHMD::DestroySwapTextureSet(SharedTextureHandle_t sharedTextureHandle)
{
	//DriverLog(__FUNCTION__" Handle: %lu", sharedTextureHandle);
	if (sharedTextureHandle)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTextureMapLock, 10000))
		{
			for (auto iter = m_TextureSets.begin(); iter != m_TextureSets.end(); iter++)
			{
				auto pSet = (TextureSet *)*iter;
				{
					if (pSet->HasHandle((HANDLE)sharedTextureHandle))
					{
						for (auto i = 0; i < 3; i++)
						{
							//if (pSet->Data[i].hSharedHandle) CloseHandle(pSet->Data[i].hSharedHandle); //already closed by openvr?
							m_TextureMap.erase((SharedTextureHandle_t)pSet->Data[i].hSharedHandle);
							pSet->Data[i].hSharedHandle = nullptr;
							SAFE_RELEASE(pSet->Data[i].pGPUResource);
							SAFE_RELEASE(pSet->Data[i].pGPUTexture);
							SAFE_RELEASE(pSet->Data[i].pCPUResource);
							SAFE_RELEASE(pSet->Data[i].pCPUTexture);
							//SAFE_RELEASE(pSet->Data[i].pScratchImage);
							//pSet->Data[i].pImage = nullptr;
						}
						delete pSet;
						pSet = nullptr;
						m_TextureSets.erase(iter);
						break;
					}
				}
			}
			ReleaseMutex(m_hTextureMapLock);
		}
	}
}

void CTrackedHMD::DestroyAllSwapTextureSets(uint32_t unPid)
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTextureMapLock, 10000))
	{
		//DriverLog(__FUNCTION__" PID: %u", unPid);
		for (auto iter = m_TextureSets.begin(); iter != m_TextureSets.end(); iter++)
		{
			auto pSet = (TextureSet *)*iter;
			if (pSet->Pid == unPid)
			{
				for (auto i = 0; i < 3; i++)
				{

					//if (pSet->Data[i].hSharedHandle) CloseHandle(pSet->Data[i].hSharedHandle);
					m_TextureMap.erase((SharedTextureHandle_t)pSet->Data[i].hSharedHandle);
					pSet->Data[i].hSharedHandle = nullptr;
					SAFE_RELEASE(pSet->Data[i].pGPUResource);
					SAFE_RELEASE(pSet->Data[i].pGPUTexture);
					SAFE_RELEASE(pSet->Data[i].pCPUResource);
					SAFE_RELEASE(pSet->Data[i].pCPUTexture);
					//SAFE_RELEASE(pSet->Data[i].pScratchImage);
					//pSet->Data[i].pImage = nullptr;
				}
				delete pSet;
				pSet = nullptr;
				m_TextureSets.erase(iter);
				break;
			}
		}
		ReleaseMutex(m_hTextureMapLock);
	}
}

void CTrackedHMD::GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2])
{	
	//DriverLog(__FUNCTION__" hTex1: %lu, hTex2: %lu, pIndices: %p", sharedTextureHandles[0], sharedTextureHandles[1], pIndices);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTextureMapLock, 10000))
	{
		for (auto i = 0; i < 2; i++)
		{
			auto iter = m_TextureMap.find(sharedTextureHandles[i]);
			if (iter == m_TextureMap.end())
				continue;
			auto tl = iter->second;
			(*pIndices)[i] = tl.pData->Index;
		}
		ReleaseMutex(m_hTextureMapLock);
	}
}

void CTrackedHMD::SubmitLayer(vr::SharedTextureHandle_t sharedTextureHandles[2], const vr::VRTextureBounds_t(&bounds)[2], const vr::HmdMatrix34_t *pPose)
{		
	if (!m_DirectScreen.FrameTime) return;
	DWORD now = GetTickCount();
	if (now - m_DirectScreen.LastFrameTime < m_DirectScreen.FrameTime)
		return;
	m_DirectScreen.LastFrameTime = now;
	//DriverLog(__FUNCTION__" hTex1: %lu, hTex2: %lu", sharedTextureHandles[0], sharedTextureHandles[1]);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTextureMapLock, 10))
	{
		auto iterLeft = m_TextureMap.find(sharedTextureHandles[0]);
		if (iterLeft != m_TextureMap.end())
		{
			auto tlLeft = &iterLeft->second;
			//tlLeft->pData->Eye = EVREye::Eye_Left;
			m_pContext->CopyResource(tlLeft->pData->pCPUResource, tlLeft->pData->pGPUResource);
			m_DirectScreen.Left.pData = tlLeft->pData;
		}
		auto iterRight = m_TextureMap.find(sharedTextureHandles[1]);
		if (iterRight != m_TextureMap.end())
		{
			auto tlRight = &iterRight->second;
			//tlRight->pData->Eye = EVREye::Eye_Right;
			m_pContext->CopyResource(tlRight->pData->pCPUResource, tlRight->pData->pGPUResource);
			m_DirectScreen.Right.pData = tlRight->pData;
		}
		ReleaseMutex(m_hTextureMapLock);
	}
}

void CTrackedHMD::Present(vr::SharedTextureHandle_t syncTexture)
{	
	return;
	//DriverLog(__FUNCTION__" syncTexture: %lu", syncTexture);

	if (m_SyncTexture != syncTexture)
	{
		m_SyncTexture = syncTexture;

		if (m_pSyncTexture)
		{
			m_pSyncTexture->Release();
			m_pSyncTexture = nullptr;
		}

		if (m_SyncTexture)
			m_pDevice->OpenSharedResource((HANDLE)m_SyncTexture, __uuidof(ID3D11Texture2D), (void **)&m_pSyncTexture);
	}


	IDXGIKeyedMutex *pSyncMutex = NULL;
	if (m_pSyncTexture != NULL && SUCCEEDED(m_pSyncTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void **)&pSyncMutex)))
	{
		if (SUCCEEDED(pSyncMutex->AcquireSync(0, 10)))
		{
			//what to do here?

			pSyncMutex->ReleaseSync(0);
		}
		pSyncMutex->Release();
	}
}

void CTrackedHMD::RunFrame(DWORD currTick)
{
	//const float step = 0.0001f;
	//if (m_KeyDown && currTick - m_LastDown > m_Delay)
	//{
	//	m_KeyDown = false;
	//}

	//if (VKD(VK_LCONTROL))
	//{
	//	if (!m_KeyDown)
	//	{
	//		if (VKD(VK_ADD))
	//		{
	//			m_HMDData.IPDValue += step;
	//			m_pDriverHost->PhysicalIpdSet(m_unObjectId, m_HMDData.IPDValue);
	//			m_pSettings->SetFloat("driver_customhmd", "IPD", m_HMDData.IPDValue);
	//			m_pSettings->Sync(true);
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_SUBTRACT))
	//		{
	//			m_HMDData.IPDValue -= step;
	//			m_pDriverHost->PhysicalIpdSet(m_unObjectId, m_HMDData.IPDValue);
	//			m_pSettings->SetFloat("driver_customhmd", "IPD", m_HMDData.IPDValue);
	//			m_pSettings->Sync(true);
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_HOME))
	//		{
	//			auto euler = Quaternion((float *)&m_HMDData.LastState.Rotation).ToEuler();
	//			euler.v[0] *= -1;
	//			euler.v[1] *= -1;
	//			euler.v[2] *= -1;
	//			m_pServer->AlignHMD(&euler);

	//			////zero offset gyro
	//			//if (pHandle)
	//			//{
	//			//	buf[0] = 0;
	//			//	buf[1] = 1;
	//			//	hid_write(pHandle, buf, sizeof(buf));
	//			//}

	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_END))
	//		{
	//			auto euler = HmdVector3d_t();
	//			m_pServer->AlignHMD(&euler);
	//			////reset gyro offset 
	//			//if (pHandle)
	//			//{
	//			//	buf[0] = 0;
	//			//	buf[1] = 1;
	//			//	hid_write(pHandle, buf, sizeof(buf));
	//			//}

	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_NUMPAD6))
	//		{
	//			m_HMDData.Pose.vecPosition[0] += 0.01;
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_NUMPAD4))
	//		{
	//			m_HMDData.Pose.vecPosition[0] -= 0.01;
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_NUMPAD8))
	//		{
	//			m_HMDData.Pose.vecPosition[1] += 0.01;
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_NUMPAD2))
	//		{
	//			m_HMDData.Pose.vecPosition[1] -= 0.01;
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_NUMPAD9))
	//		{
	//			m_HMDData.Pose.vecPosition[2] -= 0.01;
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_NUMPAD1))
	//		{
	//			m_HMDData.Pose.vecPosition[2] += 0.01;
	//			m_KeyDown = true;
	//		}
	//		else if (VKD(VK_NUMPAD5))
	//		{
	//			m_HMDData.Pose.vecPosition[0] = 0;
	//			m_HMDData.Pose.vecPosition[1] = 0;
	//			m_HMDData.Pose.vecPosition[2] = 0;
	//			m_KeyDown = true;
	//		}
	//		else
	//		{
	//			m_Delay = 500;
	//		}

	//		if (m_KeyDown)
	//		{
	//			m_LastDown = currTick;
	//			m_Delay /= 2;
	//			if (m_Delay < 2)
	//				m_Delay = 2;
	//		}
	//	}
	//}
	//else
	//{
	//	m_KeyDown = false;
	//	m_Delay = 500;
	//}

	DriverPose_t pose;
	pose.poseIsValid = false;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, 1000))
	{
		if (m_HMDData.PoseUpdated)
		{
			pose = m_HMDData.Pose;
			m_HMDData.PoseUpdated = false;
		}
		ReleaseMutex(m_HMDData.hPoseLock);
	}
	if (pose.poseIsValid)
		m_pDriverHost->TrackedDevicePoseUpdated(m_unObjectId, pose, sizeof(pose));
}

void CTrackedHMD::PacketReceived(USBPacket *pPacket, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos)
{
	if ((pPacket->Header.Type & 0x0F) != HMD_SOURCE)
		return;

	unsigned int now = GetTickCount();


	/*if (m_HMDData.LastIPDPress && (now - m_HMDData.LastIPDProcess >= 250))
	{
		auto diff = (now - m_HMDData.LastIPDPress) / 250.0f;
		if (diff <= 4) diff = 1.0f;
		if (diff > 20) diff = 20.0f;
		UserIpdMeters += 0.0001f * m_HMDData.LastIPDSign * diff;
		m_HMDData.LastIPDProcess = now;
		SET_PROP(Float, UserIpdMeters, );
	}*/


	if (WAIT_OBJECT_0 == WaitForSingleObject(m_HMDData.hPoseLock, INFINITE))
	{
		switch (pPacket->Header.Type & 0xF0)
		{
		case ROTATION_DATA:
		{
			m_HMDData.LastState.Rotation = pPacket->Rotation;
			auto euler = Quaternion((float *)&m_HMDData.LastState.Rotation).ToEuler();
			euler.v[0] = euler.v[0] + pCenterEuler->v[0];
			euler.v[1] = euler.v[1] + pCenterEuler->v[1];
			euler.v[2] = euler.v[2] + pCenterEuler->v[2];
			m_HMDData.Pose.qRotation = Quaternion::FromEuler(euler).UnitQuaternion();
			m_HMDData.PoseUpdated = true;
			memcpy(pRelativePos, m_HMDData.Pose.vecPosition, sizeof(HmdVector3d_t)); //to server driver
		}
		break;
		case POSITION_DATA:
		{
			m_HMDData.LastState.Position = pPacket->Position;
			for (auto i = 0; i < 3; i++)
				m_HMDData.Pose.vecPosition[i] = m_HMDData.LastState.Position.Position[i];
			m_HMDData.PoseUpdated = true;
		}
		break;
		case COMMAND_DATA:
			if (pPacket->Command.Command == CMD_IPD)
			{
				UserIpdMeters += 0.001f * ((float)pPacket->Command.Data.IPD.Direction);
				SET_PROP(Float, UserIpdMeters, );
			}

			break;
		case TRIGGER_DATA:
		{
			//handle IPD and seated pos center button here
			if ((pPacket->Trigger.Digital & BUTTON_0) == BUTTON_0)
			{
				//seated center 
			}
			//bool ipdState = false;
			//if ((pPacket->Trigger.Digital & BUTTON_1) == BUTTON_1)
			//{
			//	//down
			//	ipdState = true;
			//	if (!m_HMDData.LastIPDPress)
			//	{
			//		m_HMDData.LastIPDSign = -1.0f;
			//		UserIpdMeters -= 0.0001f;
			//		m_HMDData.LastIPDProcess = m_HMDData.LastIPDPress = now;
			//		SET_PROP(Float, UserIpdMeters, );
			//	}
			//}
			//if ((pPacket->Trigger.Digital & BUTTON_2) == BUTTON_2)
			//{
			//	//up
			//	ipdState = true;
			//	if (!m_HMDData.LastIPDPress)
			//	{
			//		m_HMDData.LastIPDSign = 1.0f;
			//		UserIpdMeters += 0.0001f;
			//		m_HMDData.LastIPDProcess = m_HMDData.LastIPDPress = now;
			//		SET_PROP(Float, UserIpdMeters, );
			//	}
			//}
			//if (!ipdState)
			//	m_HMDData.LastIPDPress = 0;				
		}
		break;
		}
		ReleaseMutex(m_HMDData.hPoseLock);
	}

}


#define FRAME_BUFFER_COUNT 1

bool CTrackedHMD::GetCameraFrameDimensions(ECameraVideoStreamFormat nVideoStreamFormat, uint32_t *pWidth, uint32_t *pHeight)
{
	DriverLog(__FUNCTION__" fmt: %d", nVideoStreamFormat);
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
	DriverLog(__FUNCTION__);
	*pDefaultFrameQueueSize = FRAME_BUFFER_COUNT;
	*pFrameBufferDataSize = m_Camera.Options.BufferSize;
	return true;
}

bool CTrackedHMD::SetCameraFrameBuffering(int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize)
{
	auto ppBuffers = (CameraVideoStreamFrame_t**)ppFrameBuffers;
	auto pFirstBuffer = ppBuffers[0];
	DriverLog(__FUNCTION__" fc: %d, ds: %d, pi: %p", nFrameBufferCount, nFrameBufferDataSize, pFirstBuffer->m_pImageData);
	m_Camera.pFrameBuffer = pFirstBuffer;
	m_Camera.Options.pCaptureBuffer = (pFirstBuffer + 1);
	return true;
}

bool CTrackedHMD::SetCameraVideoStreamFormat(ECameraVideoStreamFormat nVideoStreamFormat)
{
	DriverLog(__FUNCTION__" fmt: %d", nVideoStreamFormat);
	if (CVS_FORMAT_NV12 != nVideoStreamFormat)
		return false;
	m_Camera.StreamFormat = nVideoStreamFormat;
	return true;
}

ECameraVideoStreamFormat CTrackedHMD::GetCameraVideoStreamFormat()
{
	DriverLog(__FUNCTION__);
	m_Camera.StreamFormat = CVS_FORMAT_NV12;
	return m_Camera.StreamFormat;
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
	DriverLog(__FUNCTION__" en: %d", bEnable);
	return true;
}

bool CTrackedHMD::GetCameraDistortion(float flInputU, float flInputV, float *pflOutputU, float *pflOutputV)
{
	//_LOG(__FUNCTION__" iu: %f, iv: %f ou: %f, ov: %f", flInputU, flInputV, *pflOutputU, *pflOutputV);
	*pflOutputU = flInputU;
	*pflOutputV = flInputV;
	return true;
}

bool CTrackedHMD::GetCameraProjection(vr::EVRTrackedCameraFrameType eFrameType, float flZNear, float flZFar, vr::HmdMatrix44_t *pProjection)
{
	DriverLog(__FUNCTION__" ft: %d, n: %f, f: %f", eFrameType, flZNear, flZFar);
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

//bool CTrackedHMD::GetRecommendedCameraUndistortion(uint32_t *pUndistortionWidthPixels, uint32_t *pUndistortionHeightPixels)
//{
//	_LOG(__FUNCTION__);
//	*pUndistortionWidthPixels = m_Camera.Options.Width;
//	*pUndistortionHeightPixels = m_Camera.Options.Height;
//	return true;
//}
//
//bool CTrackedHMD::SetCameraUndistortion(uint32_t nUndistortionWidthPixels, uint32_t nUndistortionHeightPixels)
//{
//	_LOG(__FUNCTION__" w: %d, h: %d", nUndistortionWidthPixels, nUndistortionHeightPixels);
//	return true;
//}

bool CTrackedHMD::SetFrameRate(int nISPFrameRate, int nSensorFrameRate)
{
	DriverLog(__FUNCTION__" ifr: %d, sfr: %d ", nISPFrameRate, nSensorFrameRate);
	return true;
}

bool CTrackedHMD::SetCameraVideoSinkCallback(ICameraVideoSinkCallback *pCameraVideoSinkCallback)
{
	DriverLog(__FUNCTION__" cb: %p", pCameraVideoSinkCallback);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_Camera.hLock, INFINITE))
	{
		m_Camera.pfCallback = pCameraVideoSinkCallback;
		ReleaseMutex(m_Camera.hLock);
	}
	return true;
}

bool CTrackedHMD::GetCameraCompatibilityMode(ECameraCompatibilityMode *pCameraCompatibilityMode)
{
	DriverLog(__FUNCTION__);
	*pCameraCompatibilityMode = CAMERA_COMPAT_MODE_ISO_30FPS;
	return true;
}

bool CTrackedHMD::SetCameraCompatibilityMode(ECameraCompatibilityMode nCameraCompatibilityMode)
{
	DriverLog(__FUNCTION__" cm: %d", nCameraCompatibilityMode);
	if (nCameraCompatibilityMode == CAMERA_COMPAT_MODE_ISO_30FPS) return true;
	return false;
}

bool CTrackedHMD::GetCameraFrameBounds(EVRTrackedCameraFrameType eFrameType, uint32_t *pLeft, uint32_t *pTop, uint32_t *pWidth, uint32_t *pHeight)
{
	DriverLog(__FUNCTION__" ft: %d", eFrameType);
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
	DriverLog(__FUNCTION__" ft: %d", eFrameType);
	return false;
}

void CTrackedHMD::SetupCamera()
{
	DriverLog(__FUNCTION__);
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
	DriverLog(__FUNCTION__);
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
	DriverLog(__FUNCTION__);
	if (m_Camera.pCaptureDevice)
		m_Camera.pCaptureDevice->Stop();
}

bool CTrackedHMD::IsVideoStreamActive(bool *pbPaused, float *pflElapsedTime)
{
	auto result = m_Camera.pCaptureDevice && (m_Camera.pCaptureDevice->m_Status != CCaptureDevice::Stopped);
	DriverLog(__FUNCTION__" returning %d", result);
	if (result && pbPaused)
		*pbPaused = m_Camera.pCaptureDevice->m_Status == CCaptureDevice::Paused;

	if (pflElapsedTime)
		*pflElapsedTime = (float)(GetTickCount() - m_Camera.StartTime) / 1000.0f;
	return result;
}


bool CTrackedHMD::PauseVideoStream()
{
	DriverLog(__FUNCTION__);
	if (m_Camera.pCaptureDevice)
		return m_Camera.pCaptureDevice->Pause();
	return false;
}

bool CTrackedHMD::ResumeVideoStream()
{
	DriverLog(__FUNCTION__);
	if (m_Camera.pCaptureDevice)
		return m_Camera.pCaptureDevice->Resume();
	return false;
}

void CTrackedHMD::CameraFrameUpdateCallback(CCaptureDevice *pDevice, void *pUserData)
{
	auto pThis = (CTrackedHMD *)pUserData;
	pThis->OnCameraFrameUpdate();
}

void CTrackedHMD::OnCameraFrameUpdate()
{
	//DriverLog(__FUNCTION__);
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_Camera.hLock, INFINITE))
	{
		if (m_Camera.pCaptureDevice->m_Status == CCaptureDevice::Started && m_Camera.pfCallback && m_Camera.pFrameBuffer)
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

BOOL CALLBACK CTrackedHMD::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	auto pMonData = (HMDData *)dwData;
	pMonData->pHMDDriver->DriverLog("Monitor Enumeration callback..");

	MONITORINFOEX monInfo = {};
	monInfo.cbSize = sizeof(monInfo);
	wchar_t DeviceID[4096] = {};
	if (GetMonitorInfo(hMonitor, &monInfo))
	{
		DISPLAY_DEVICE ddMon;
		ZeroMemory(&ddMon, sizeof(ddMon));
		ddMon.cb = sizeof(ddMon);
		DWORD devMon = 0;
		pMonData->pHMDDriver->DriverLog("Enumerating monitors for %S...", monInfo.szDevice);
		while (EnumDisplayDevices(monInfo.szDevice, devMon, &ddMon, 0))
		{
			pMonData->pHMDDriver->DriverLog("Checking %S...", ddMon.DeviceID);
			if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE && !(ddMon.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
			{
				wsprintf(DeviceID, L"%S", ddMon.DeviceID);
				wchar_t *pStart = wcschr(DeviceID, L'\\');
				if (pStart)
				{
					pStart++;
					wchar_t *pEnd = wcschr(pStart, L'\\');
					if (pEnd)
					{
						*pEnd = 0;
						if (!wcscmp(pStart, pMonData->Model)) //look for this monitor id (SonyHMZ-T2)
						{
							wcscpy_s(pMonData->DisplayName, monInfo.szDevice);
							pMonData->PosX = monInfo.rcMonitor.left;
							pMonData->PosY = monInfo.rcMonitor.top;
							pMonData->ScreenWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
							pMonData->ScreenHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
							if (pMonData->ScreenWidth == 1280 && pMonData->ScreenHeight == 1470)
							{
								pMonData->FakePackDetected = true;
								pMonData->AspectRatio = ((float)(pMonData->ScreenHeight - 30) / 2.0f) / (float)pMonData->ScreenWidth;
							}
							else
							{
								pMonData->FakePackDetected = false;
								pMonData->AspectRatio = (float)pMonData->ScreenWidth / (float)pMonData->ScreenHeight;
							}
							DEVMODE devMode = {};
							devMode.dmSize = sizeof(DEVMODE);
							if (EnumDisplaySettings(monInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
								pMonData->Frequency = (float)devMode.dmDisplayFrequency;
							pMonData->IsConnected = true;

							pMonData->pHMDDriver->DriverLog("Found monitor %S.", pMonData->DisplayName);
							return FALSE;
						}
					}
				}
			}
			devMon++;

			ZeroMemory(&ddMon, sizeof(ddMon));
			ddMon.cb = sizeof(ddMon);
		}
		pMonData->pHMDDriver->DriverLog("No more monitors!");
	}
	return TRUE;
}

