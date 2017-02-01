#include "WatchDogDriver.h"
#include "ShMem.h"
//#include "simplejson\JSONValue.h"

CWatchDogDriver::CWatchDogDriver()
{
	m_pWatchdogThread = nullptr;
	m_Running = false;
}

CWatchDogDriver::~CWatchDogDriver()
{
	//Cleanup();
}

//EVRInitError CWatchDogDriver::Init(IVRDriverLog * pDriverLog, IWatchDogDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
EVRInitError CWatchDogDriver::Init(vr::IVRDriverContext *pDriverContext) // EWatchDogDriverMode eDriverMode, IVRDriverLog *pDriverLog, vr::IWatchDogDriverHost *pDriverHost, const char *pchUserDriverConfigDir, const char *pchDriverInstallDir)
{
	VR_INIT_WATCHDOG_DRIVER_CONTEXT(pDriverContext);
	InitDriverLog(vr::VRDriverLog());
	DriverLog(__FUNCTION__);
	//
	//if (eDriverMode == EWatchDogDriverMode::WatchDogDriverMode_Watchdog)
	//	return VRInitError_Init_NotAvailableToWatchdogApps;
	//m_pLog = new CDriverLog(vr::VRDriverLog());
	//_LOG(__FUNCTION__" Conf: %s  Driver: %s", pchUserDriverConfigDir, pchDriverInstallDir);
	m_pDriverHost = vr::VRWatchdogHost();
	//m_UserDriverConfigDir = pchUserDriverConfigDir;
	//m_DriverInstallDir = pchDriverInstallDir;
	m_pSettings = vr::VRSettings(); // (IVRSettings *) m_pDriverHost->GetGenericInterface(IVRSettings_Version);

	m_Running = true;
	m_pWatchdogThread = new std::thread(WatchdogThreadFunction, this);
	if (!m_pWatchdogThread)
	{
		DriverLog("Unable to create watchdog thread\n");
		return VRInitError_Driver_Failed;
	}	
	return VRInitError_None;
}

void CWatchDogDriver::Cleanup()
{
	DriverLog(__FUNCTION__);
	m_Running = false;
	if (m_pWatchdogThread)
	{
		m_pWatchdogThread->join();
		delete m_pWatchdogThread;
		m_pWatchdogThread = nullptr;
	}		
	m_pDriverHost = nullptr;
	CleanupDriverLog();
	VR_CLEANUP_WATCHDOG_DRIVER_CONTEXT();
	//m_UserDriverConfigDir.clear();
	//m_DriverInstallDir.clear();
	//delete m_pLog;
}

void CWatchDogDriver::WatchdogThreadFunction(CWatchDogDriver *pWatchDog)
{	
	CShMem shMem;
	pWatchDog->DriverLog(__FUNCTION__);
	auto lastState = CommState::Disconnected;
	while (pWatchDog->m_Running)
	{
		auto state = shMem.GetState();
		if (!shMem.WatchDogEnabled)
		{
			lastState = CommState::Disconnected;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}
		if (state != CommState::Disconnected && lastState != state)
			vr::VRWatchdogHost()->WatchdogWakeUp();
		lastState = state;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

//const JSONValue *GetJson(const JSONObject object, const wchar_t *section, const wchar_t *key)
//{
//	auto s = object.find(section);
//	if (s == object.end()) return nullptr;
//	auto sec = s->second;
//	if (!sec->IsObject()) return nullptr;
//	auto k = sec->AsObject();
//	auto ke = k.find(key);
//	if (ke == k.end()) return nullptr;
//	return ke->second;
//}

//bool CWatchDogDriver::BIsHmdPresent(const char *pchUserConfigDir)
//{		
//	CShMem mem;
//	//check usb
//	//hid_init();
//	//hid_device *handle = hid_open(0x1974, 0x0001, NULL);
//	//if (!handle)
//	//	ret = false;
//	//else
//	//	hid_close(handle);
//	//hid_exit();
//
//	auto ret = mem.GetState() != CommState::Disconnected;
//
//	if (ret)
//	{
//		//why no settings interface here? :((
//		char settingsFile[MAX_PATH];
//		sprintf_s(settingsFile, "%s\\steamvr.vrsettings", pchUserConfigDir);
//
//		FILE *fp = nullptr;
//		if (!fopen_s(&fp, settingsFile, "rb") && fp)
//		{
//			// Determine file size
//			fseek(fp, 0, SEEK_END);
//			size_t size = ftell(fp);
//
//			char *pData = new char[size + 1];
//			rewind(fp);
//			fread(pData, sizeof(char), size, fp);
//			fclose(fp);
//			fp = nullptr;
//			pData[size] = 0;
//
//			auto json = JSON::Parse(pData);
//			delete[] pData;
//			pData = nullptr;
//
//			if (json)
//			{
//				if (json->IsObject())
//				{
//					HMDData monData = {};
//					auto value = GetJson(json->AsObject(), L"steamvr", L"directMode");
//					monData.DirectMode = true; //direct mode is enabled by default
//					if (value && value->IsBool())
//						monData.DirectMode = value->AsBool();
//					if (monData.DirectMode)
//					{
//						//check if direct mode monitor is connected
//						value = GetJson(json->AsObject(), L"steamvr", L"directModeEdidVid");
//						uint16_t edid = (value && value->IsNumber()) ? (uint16_t)value->AsNumber() : 0;
//						if (edid)
//							ret &= IsD2DConnected(edid);
//						else
//							monData.DirectMode = false;
//					}
//					if (!monData.DirectMode)
//					{
//						//check extendedmode if monitor is connected						
//						monData.Model[0] = 0;
//
//						value = GetJson(json->AsObject(), L"driver_customhmd", L"monitor");
//
//						if (value && value->IsString())
//							wcscpy_s(monData.Model, value->AsString().c_str());
//
//						if (monData.Model[0])
//							EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&monData);
//						ret &= monData.IsConnected;
//					}
//				}
//				delete json;
//				json = nullptr;
//			}
//		}
//
//	}
//
//	return ret;
//}
//
//EVRInitError CWatchDogDriver::SetDisplayId(const char * pchDisplayId)
//{
//	_LOG(__FUNCTION__" %s", pchDisplayId);
//	return VRInitError_None;
//}
//
//HiddenAreaMesh_t CWatchDogDriver::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type)
//{
//	_LOG(__FUNCTION__" Eye: %d", eEye);
//	HiddenAreaMesh_t hidden_area_mesh;
//	hidden_area_mesh.pVertexData = nullptr;
//	hidden_area_mesh.unTriangleCount = 0;
//	return hidden_area_mesh;
//}
//
//uint32_t CWatchDogDriver::GetMCImage(uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen)
//{
//	_LOG(__FUNCTION__);
//	return 0;
//}
