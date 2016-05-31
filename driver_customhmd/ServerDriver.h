#ifndef ServerDriver_H
#define ServerDriver_H

#include "Common.h"	
#include "hidapi.h"

using namespace vr;
class CTrackedDevice;

class CServerDriver : 
	public IServerTrackedDeviceProvider
{		
private:	
	EVRInitError Init(IDriverLog * pDriverLog, IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir) override;
	void Cleanup() override;
	uint32_t GetTrackedDeviceCount() override;
	ITrackedDeviceServerDriver * GetTrackedDeviceDriver(uint32_t unWhich) override;
	ITrackedDeviceServerDriver * FindTrackedDeviceDriver(const char * pchId) override;
	void RunFrame() override;
	bool ShouldBlockStandbyMode() override;
	void EnterStandby() override;
	void LeaveStandby() override;
	const char * const * GetInterfaceVersions() override;
public:
	CDriverLog *m_pLog;
	IServerDriverHost *m_pDriverHost;
	void AlignHMD(HmdVector3d_t *pAlign);
private:
	std::string m_UserDriverConfigDir;
	std::string m_DriverInstallDir;
	IVRSettings *m_pSettings;
	HmdVector3d_t m_Align;
	std::vector<CTrackedDevice *> m_TrackedDevices;
	HANDLE m_hThread;
	bool m_IsRunning;
	unsigned int static WINAPI ProcessThread(void *p);
	void Run();
	void OpenUSB(hid_device **ppHandle);
	void CloseUSB(hid_device **ppHandle);
	DWORD m_LastTick, m_CurrTick;
	
};

#endif // ServerDriver_H