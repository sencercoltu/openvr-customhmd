#ifndef ClientDriver_H
#define ClientDriver_H

#include "Common.h"

using namespace vr;

class CClientDriver : 
	public IClientTrackedDeviceProvider
{
public:
	virtual EVRInitError Init(EClientDriverMode eDriverMode, IDriverLog *pDriverLog, vr::IClientDriverHost *pDriverHost, const char *pchUserDriverConfigDir, const char *pchDriverInstallDir) override;
	//virtual EVRInitError Init(IDriverLog * pDriverLog, IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir) override;
	virtual void Cleanup() override;
	virtual bool BIsHmdPresent(const char * pchUserConfigDir) override;
	virtual EVRInitError SetDisplayId(const char * pchDisplayId) override;
	virtual HiddenAreaMesh_t GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) override;
	virtual uint32_t GetMCImage(uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen) override;	
private:	
	//IVRSettings *m_pSettings;
	CDriverLog* m_pLog;
	IClientDriverHost* m_pDriverHost;
	std::string m_UserDriverConfigDir;
	std::string m_DriverInstallDir;		
};
#endif // ClientDriver_H