#include "ClientDriver.h"
#include "hidapi.h"
#include "simplejson\JSONValue.h"

//EVRInitError CClientDriver::Init(IDriverLog * pDriverLog, IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
EVRInitError CClientDriver::Init(EClientDriverMode eDriverMode, IDriverLog *pDriverLog, vr::IClientDriverHost *pDriverHost, const char *pchUserDriverConfigDir, const char *pchDriverInstallDir)
{
	if (eDriverMode == EClientDriverMode::ClientDriverMode_Watchdog)
		return VRInitError_Init_NotAvailableToWatchdogApps;
	m_pLog = new CDriverLog(pDriverLog);
	_LOG(__FUNCTION__" Conf: %s  Driver: %s", pchUserDriverConfigDir, pchDriverInstallDir);
	m_pDriverHost = pDriverHost;
	m_UserDriverConfigDir = pchUserDriverConfigDir;
	m_DriverInstallDir = pchDriverInstallDir;
	//m_pSettings = m_pDriverHost->GetSettings(IVRSettings_Version);
	return VRInitError_None;
}

void CClientDriver::Cleanup()
{
	_LOG(__FUNCTION__);
	m_pDriverHost = nullptr;
	m_UserDriverConfigDir.clear();
	m_DriverInstallDir.clear();
	delete m_pLog;
}

const JSONValue *GetJson(const JSONObject object, const wchar_t *section, const wchar_t *key)
{
	auto s = object.find(section);
	if (s == object.end()) return nullptr;
	auto sec = s->second;
	if (!sec->IsObject()) return nullptr;
	auto k = sec->AsObject();
	auto ke = k.find(key);
	if (ke == k.end()) return nullptr;
	return ke->second;
}

bool CClientDriver::BIsHmdPresent(const char *pchUserConfigDir)
{
	
	auto ret = true;
	//check usb
	hid_init();
	hid_device *handle = hid_open(0x1974, 0x0001, NULL);
	if (!handle)
		ret = false;
	else
		hid_close(handle);
	hid_exit();

	if (ret)
	{
		//why no settings interface here? :((
		char settingsFile[MAX_PATH];
		sprintf_s(settingsFile, "%s\\steamvr.vrsettings", pchUserConfigDir);

		FILE *fp = nullptr;
		if (!fopen_s(&fp, settingsFile, "rb") && fp)
		{
			// Determine file size
			fseek(fp, 0, SEEK_END);
			size_t size = ftell(fp);

			char *pData = new char[size + 1];
			rewind(fp);
			fread(pData, sizeof(char), size, fp);
			fclose(fp);
			fp = nullptr;
			pData[size] = 0;

			auto json = JSON::Parse(pData);
			delete[] pData;
			pData = nullptr;

			if (json)
			{
				if (json->IsObject())
				{
					HMDData monData = {};
					auto value = GetJson(json->AsObject(), L"steamvr", L"directMode");
					monData.DirectMode = true; //direct mode is enabled by default
					if (value && value->IsBool())
						monData.DirectMode = value->AsBool();
					if (monData.DirectMode)
					{
						//check if direct mode monitor is connected
						value = GetJson(json->AsObject(), L"steamvr", L"directModeEdidVid");
						uint16_t edid = (value && value->IsNumber()) ? (uint16_t)value->AsNumber() : 0;
						if (edid)
							ret &= IsD2DConnected(edid);
						else
							monData.DirectMode = false;
					}
					if (!monData.DirectMode)
					{
						//check extendedmode if monitor is connected
						wcscpy_s(monData.Model, L"SNYD602");						
						value = GetJson(json->AsObject(), L"driver_customhmd", L"monitor");
						
						if (value && value->IsString())
						{							
							//std::string basic_string(value);
							//std::wstring wchar_value(basic_string.begin(), basic_string.end());
							//wcscpy_s(monData.Model, wchar_value.c_str());
							wcscpy_s(monData.Model, value->AsString().c_str());
						}
						EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&monData);
						ret &= monData.IsConnected;
					}
				}
				delete json;
				json = nullptr;
			}
		}

	}

	return ret;
}

EVRInitError CClientDriver::SetDisplayId(const char * pchDisplayId)
{
	_LOG(__FUNCTION__" %s", pchDisplayId);
	return VRInitError_None;
}

HiddenAreaMesh_t CClientDriver::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type)
{
	_LOG(__FUNCTION__" Eye: %d", eEye);
	HiddenAreaMesh_t hidden_area_mesh;
	hidden_area_mesh.pVertexData = nullptr;
	hidden_area_mesh.unTriangleCount = 0;
	return hidden_area_mesh;
}

uint32_t CClientDriver::GetMCImage(uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen)
{
	_LOG(__FUNCTION__);
	return 0;
}
