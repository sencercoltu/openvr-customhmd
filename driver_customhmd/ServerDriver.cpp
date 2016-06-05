#include "ServerDriver.h"
#include "TrackedHMD.h"
#include "TrackedController.h"
#include <process.h>

EVRInitError CServerDriver::Init(IDriverLog * pDriverLog, IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{		
	m_CurrTick = m_LastTick = GetTickCount();

	m_pLog = new CDriverLog(pDriverLog);

	_LOG(__FUNCTION__" start");

	m_pDriverHost = pDriverHost;
	m_UserDriverConfigDir = pchUserDriverConfigDir;
	m_DriverInstallDir = pchDriverInstallDir;

	m_pSettings = pDriverHost ? pDriverHost->GetSettings(IVRSettings_Version) : nullptr;
	m_Align = { 0 };
	m_Relative = { 0 };
	if (m_pSettings)
	{
		m_Align.v[0] = m_pSettings->GetFloat("driver_customhmd", "eoX", 0.0f);
		m_Align.v[1] = m_pSettings->GetFloat("driver_customhmd", "eoY", 0.0f);
		m_Align.v[2] = m_pSettings->GetFloat("driver_customhmd", "eoZ", 0.0f);		
	}
	
	m_TrackedDevices.push_back(new CTrackedHMD("HMD", this));
	m_TrackedDevices.push_back(new CTrackedController(TrackedControllerRole_RightHand, "RIGHT CONTROLLER", this));
	m_TrackedDevices.push_back(new CTrackedController(TrackedControllerRole_LeftHand, "LEFT CONTROLLER", this));

	m_hThread = nullptr;
	m_IsRunning = false;

	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ProcessThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hThread);
	}

	_LOG(__FUNCTION__" end");
	return VRInitError_None;
}

void CServerDriver::Cleanup()
{
	m_IsRunning = false;
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}

	_LOG(__FUNCTION__);
	for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
		delete (*iter);
	m_TrackedDevices.clear();

	m_pDriverHost = nullptr;
	m_UserDriverConfigDir.clear();
	m_DriverInstallDir.clear();
	delete m_pLog;
	m_pLog = nullptr;
}

unsigned int WINAPI CServerDriver::ProcessThread(void *p)
{
	auto serverDriver = static_cast<CServerDriver *>(p);
	if (serverDriver)					
		serverDriver->Run();	
	_endthreadex(0);
	return 0;
}

void CServerDriver::OpenUSB(hid_device **ppHandle)
{
	CloseUSB(ppHandle);
	hid_device *handle = hid_open(0x104d, 0x1974, nullptr);
	if (!handle)
		return;
	*ppHandle = handle;
	hid_set_nonblocking(handle, 1);
}

void CServerDriver::CloseUSB(hid_device **ppHandle)
{
	if (!ppHandle || !*ppHandle)
		return;
	hid_close(*ppHandle);
	*ppHandle = nullptr;
}

void CServerDriver::Run()
{	
	int pos = 0;
	int res;

	hid_init();
	
	long count = 0;

	unsigned char buf[65] = { 0 };

	USBData *pUSBData = (USBData *)buf;

	hid_device *pHandle = nullptr;
	DWORD lastTick = GetTickCount();

	while (m_IsRunning)
	{
		//pOverlayManager->HandleEvents();
		if (!pHandle)
		{
			OpenUSB(&pHandle);
			lastTick = GetTickCount();
			if (!pHandle)
			{
				//MessageBox(nullptr, L"No USB", L"Info", 0);
				Sleep(1000);
				continue;
			}
		}
		else
		{
			res = hid_read_timeout(pHandle, buf, sizeof(buf), 10);
			if (res > 0)
			{
				lastTick = GetTickCount();				
				for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
					(*iter)->PoseUpdate(pUSBData, &m_Align, &m_Relative);
			}
			else if (res < 0)
			{
				//usb fucked up?				
				//MessageBox(nullptr, L"Disco", L"Info", 0);
				CloseUSB(&pHandle);
				Sleep(1000);
			}
			else
			{
				if (GetTickCount() - lastTick >= 5000)
				{
					CloseUSB(&pHandle);
					Sleep(1000);
				}
			}
			Sleep(1);
		}

		//m_pDriverHost->TrackedDevicePoseUpdated(m_unObjectId, m_HMDData.Pose);
	}

	CloseUSB(&pHandle);
	hid_exit();	

}

uint32_t CServerDriver::GetTrackedDeviceCount()
{
	_LOG(__FUNCTION__" returns %d", m_TrackedDevices.size());
	return (uint32_t)m_TrackedDevices.size();
}

ITrackedDeviceServerDriver * CServerDriver::GetTrackedDeviceDriver(uint32_t unWhich)
{
	_LOG(__FUNCTION__" idx: %d", unWhich);
	//if (0 != _stricmp(pchInterfaceVersion, ITrackedDeviceServerDriver_Version))
	//	return nullptr;	
	if (unWhich >= m_TrackedDevices.size())
		return nullptr;
	return m_TrackedDevices.at(unWhich);
}

ITrackedDeviceServerDriver * CServerDriver::FindTrackedDeviceDriver(const char * pchId)
{
	_LOG(__FUNCTION__" id: %s", pchId);
	for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
	{
		if (0 == std::strcmp(pchId, (*iter)->Prop_SerialNumber.c_str()))
		{ 
			return *iter;
		}
	}
	return nullptr;
}

void CServerDriver::RunFrame()
{
	DWORD currTick = GetTickCount();
	for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
		(*iter)->RunFrame(currTick);
}

bool CServerDriver::ShouldBlockStandbyMode()
{
	_LOG(__FUNCTION__);
	return false;
}

void CServerDriver::EnterStandby()
{
	_LOG(__FUNCTION__);
}

void CServerDriver::LeaveStandby()
{
	_LOG(__FUNCTION__);
}

const char * const * CServerDriver::GetInterfaceVersions()
{
	return k_InterfaceVersions;
}

void CServerDriver::AlignHMD(HmdVector3d_t *pAlign)
{
	m_Align = *pAlign;
	if (m_pSettings)
	{
		m_pSettings->SetFloat("driver_customhmd", "eoX", (float)m_Align.v[0]);
		m_pSettings->SetFloat("driver_customhmd", "eoY", (float)m_Align.v[1]);
		m_pSettings->SetFloat("driver_customhmd", "eoZ", (float)m_Align.v[2]);
		m_pSettings->Sync(true);
	}
}