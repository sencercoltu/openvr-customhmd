#ifndef WatchDogDriver_H
#define WatchDogDriver_H

#include "Common.h"
#include <thread>

using namespace vr;

class CWatchDogDriver : 	
	public IVRWatchdogProvider,
	public CDriverLog
{
public:
	CWatchDogDriver();
	~CWatchDogDriver();
	virtual EVRInitError Init(vr::IVRDriverContext *pDriverContext); // EWatchDogDriverMode eDriverMode, IVRDriverLog *pDriverLog, vr::IWatchDogDriverHost *pDriverHost, const char *pchUserDriverConfigDir, const char *pchDriverInstallDir) override;
	//virtual EVRInitError Init(IDriverLog * pDriverLog, IWatchDogDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir) override;
	virtual void Cleanup() override;
	//virtual bool BIsHmdPresent(const char * pchUserConfigDir) override;
	//virtual EVRInitError SetDisplayId(const char * pchDisplayId) override;
	//virtual HiddenAreaMesh_t GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) override;
	//virtual uint32_t GetMCImage(uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen) override;	
private:	
	IVRSettings *m_pSettings;
	//CDriverLog* m_pLog;
	IVRWatchdogHost* m_pDriverHost;
	std::thread *m_pWatchdogThread;	
	bool m_Running;
	static void WatchdogThreadFunction(CWatchDogDriver *ptr);
	//std::string m_UserDriverConfigDir;
	//std::string m_DriverInstallDir;		
};
#endif // WatchDogDriver_H