#include "ServerDriver.h"
#include "TrackedHMD.h"
#include "TrackedController.h"
#include <process.h>
#include "ShMem.h"

EVRInitError CServerDriver::Init(IVRDriverContext *pDriverContext) //IVRDriverLog * pDriverLog, IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir)
{	
	pSharedMem = nullptr;
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	InitDriverLog(vr::VRDriverLog());

	timeBeginPeriod(1);
	m_CurrTick = m_LastTick = GetTickCount();

	//m_pLog = new CDriverLog(vr::VRDriverLog());
	m_LastTypeSequence = 0;
	DriverLog(__FUNCTION__" start");

	m_pDriverHost = vr::VRServerDriverHost();
	//m_UserDriverConfigDir = pchUserDriverConfigDir;
	//m_DriverInstallDir = pchDriverInstallDir;

	m_HMDAdded = m_RightCtlAdded = m_LeftCtlAdded = false;

	m_pSettings = vr::VRSettings(); // m_pDriverHost ? m_pDriverHost->GetSettings(IVRSettings_Version) : nullptr;
	m_Align = { 0 };
	m_Relative = { 0 };
	//if (m_pSettings)
	//{
	//	m_Align.v[0] = m_pSettings->GetFloat("driver_customhmd", "eoX");
	//	m_Align.v[1] = m_pSettings->GetFloat("driver_customhmd", "eoY");
	//	m_Align.v[2] = m_pSettings->GetFloat("driver_customhmd", "eoZ");
	//}

	 
	m_HMDAdded = true;
	auto hmd = new CTrackedHMD("HMD", this);

	if (!hmd->IsConnected())
		return VRInitError_Init_HmdNotFound;

	pSharedMem = new CShMem();

	m_TrackedDevices.push_back(hmd); //seems steamvr wont init without adding hmd device on init??	


	m_hThread = nullptr;
	m_IsRunning = false;

	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ProcessThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hThread);
	}

	DriverLog(__FUNCTION__" end");
	return VRInitError_None;
}

void CServerDriver::Cleanup()
{
	DriverLog(__FUNCTION__);
	m_IsRunning = false;
	
	SAFE_THREADCLOSE(m_hThread);

	for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
		delete (*iter);
	m_TrackedDevices.clear();

	m_pDriverHost = nullptr;
	//m_UserDriverConfigDir.clear();
	//m_DriverInstallDir.clear();
	//delete m_pLog;
	//m_pLog = nullptr;
	timeEndPeriod(1);
	CleanupDriverLog();
	VR_CLEANUP_SERVER_DRIVER_CONTEXT();

	if (pSharedMem)
		delete pSharedMem;
	pSharedMem = nullptr;
}

unsigned int WINAPI CServerDriver::ProcessThread(void *p)
{
	auto serverDriver = static_cast<CServerDriver *>(p);
	if (serverDriver)
		serverDriver->Run();
	_endthreadex(0);
	return 0;
}

bool CServerDriver::IsTrackerConnected()
{
	return pSharedMem->_status.State == CommState::TrackerActive;
}

void CServerDriver::SendDriverCommand(USBPacket *command)
{
	m_CommandQueue.push_back(command);
}

//void CServerDriver::SendScreen(char *screenData, int size)
//{
//	pSharedMem->WriteScreen(screenData, size);
//}

void CServerDriver::ScanSyncReceived(uint64_t syncTime)
{
	DriverLog(__FUNCTION__" sync @ %I64u", syncTime);
}

void CServerDriver::RemoveTrackedDevice(CTrackedDevice *pDevice)
{
	for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
	{
		if (*iter == pDevice)
		{
			delete (*iter);
			return;
		}
	}	
}

void CServerDriver::Run()
{
	int pos = 0;
	long count = 0;

	USBPacket tUSBPacket = { 0 };

	DWORD lastTick = GetTickCount();
	//DWORD lastCalibSend = 0;
	//uint8_t calibDeviceIndex = 0;

	//USBCalibrationData calibrationData[3] = { 0 };

	
	auto state = CommState::Uninitialized;
	auto prevState = state;

	while (m_IsRunning)
	{
		prevState = state;
		state = pSharedMem->GetState();
		if ((state & CommState::TrackerActive) != CommState::TrackerActive)
		{
			//just update drivertime in state object so monitor knows the driver is active
			pSharedMem->UpdateDriverTime();
			Sleep(100);
			continue;
		}

		DWORD now = GetTickCount();

		if (m_CommandQueue.size() > 0)
		{
			//buf[0] = 0x00;
			USBPacket *pPacket = m_CommandQueue.front();
			//*((USBPacket*)(buf + 1)) = *pPacket;
			pSharedMem->WriteOutgoingPacket((char *)pPacket);
			m_CommandQueue.pop_front();
			delete pPacket;
			//res = hid_write(pHandle, buf, sizeof(buf));
		}

		int total = 0;
		char *packets = pSharedMem->ReadIncomingPackets(&total);
		//res = hid_read_timeout(pHandle, buf, sizeof(buf), 10); 
		if (total == 0)
		{
			Sleep(1);
			continue;
		}

		lastTick = now;

		//auto crcTemp = pUSBPacket->Header.Crc8;
		//pUSBPacket->Header.Crc8 = 0;
		//uint8_t* data = (uint8_t*)pUSBPacket;
		//uint8_t crc = 0;
		//for (int i = 0; i<sizeof(USBPacket); i++)
		//	crc ^= data[i];
		//if (crc == crcTemp)
		for (auto i = 0; i < total; i++)
		{
			USBPacket *pUSBPacket = (USBPacket *)&packets[i * 32];
			ProcessUSBPacket(pUSBPacket);
		}

		delete packets;
	}
}

void CServerDriver::ProcessUSBPacket(USBPacket *pUSBPacket)
{
	if (CheckPacketCrc(pUSBPacket))
	{
		switch (pUSBPacket->Header.Type & 0x0F)
		{
		case HMD_SOURCE:
			if (!m_HMDAdded)
			{
				m_HMDAdded = true;
				m_TrackedDevices.push_back(new CTrackedHMD("HMD", this)); //only add hmd
			}
			break;
		case BASESTATION_SOURCE:
			if ((pUSBPacket->Header.Type & 0xF0) == COMMAND_DATA && pUSBPacket->Command.Command == CMD_SYNC)
				ScanSyncReceived(pUSBPacket->Command.Data.Sync.SyncTime);
			break;
		case LEFTCTL_SOURCE:
			if (!m_LeftCtlAdded)
			{
				m_LeftCtlAdded = true;
				m_TrackedDevices.push_back(new CTrackedController(TrackedControllerRole_LeftHand, "LEFT CONTROLLER", this));
			}
			break;
		case RIGHTCTL_SOURCE:
			if (!m_RightCtlAdded)
			{
				m_RightCtlAdded = true;
				m_TrackedDevices.push_back(new CTrackedController(TrackedControllerRole_RightHand, "RIGHT CONTROLLER", this));
			}
			break;
		}
		for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
			(*iter)->PacketReceived(pUSBPacket, &m_Align, &m_Relative);
	}
}

//uint32_t CServerDriver::GetTrackedDeviceCount()
//{
//	_LOG(__FUNCTION__" returns %d", m_TrackedDevices.size());
//	return (uint32_t)m_TrackedDevices.size();
//}
//
//ITrackedDeviceServerDriver * CServerDriver::GetTrackedDeviceDriver(uint32_t unWhich)
//{
//	_LOG(__FUNCTION__" idx: %d", unWhich);
//	//if (0 != _stricmp(pchInterfaceVersion, ITrackedDeviceServerDriver_Version))
//	//	return nullptr;	
//	if (unWhich >= m_TrackedDevices.size())
//		return nullptr;
//	return m_TrackedDevices.at(unWhich);
//}
//
//ITrackedDeviceServerDriver * CServerDriver::FindTrackedDeviceDriver(const char * pchId)
//{
//	_LOG(__FUNCTION__" id: %s", pchId);
//	for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
//	{
//		if (0 == std::strcmp(pchId, (*iter)->Prop_SerialNumber.c_str()))
//		{
//			return *iter;
//		}
//	}
//	return nullptr;
//}

void CServerDriver::RunFrame()
{
	DWORD currTick = GetTickCount();
	for (auto iter = m_TrackedDevices.begin(); iter != m_TrackedDevices.end(); iter++)
		(*iter)->RunFrame(currTick);
}

bool CServerDriver::ShouldBlockStandbyMode()
{
	DriverLog(__FUNCTION__);
	return true;
}

void CServerDriver::EnterStandby()
{
	DriverLog(__FUNCTION__);
}

void CServerDriver::LeaveStandby()
{
	DriverLog(__FUNCTION__);
}

const char * const * CServerDriver::GetInterfaceVersions()
{
	return k_InterfaceVersions;
}

void CServerDriver::AlignHMD(HmdVector3d_t *pAlign)
{
	//trash
	m_Align = *pAlign;
	if (m_pSettings)
	{
		m_pSettings->SetFloat("driver_customhmd", "eoX", (float)m_Align.v[0]);
		m_pSettings->SetFloat("driver_customhmd", "eoY", (float)m_Align.v[1]);
		m_pSettings->SetFloat("driver_customhmd", "eoZ", (float)m_Align.v[2]);
		m_pSettings->Sync(true);
	}
}