#ifndef ServerDriver_H
#define ServerDriver_H

#include "Common.h"	
#include "TrackedDevice.h"


using namespace vr;
class CServerDriver : public IServerTrackedDeviceProvider
{	
public:
	virtual EVRInitError Init(IDriverLog * pDriverLog, IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir) override;
	virtual void Cleanup() override;
	virtual uint32_t GetTrackedDeviceCount() override;
	virtual ITrackedDeviceServerDriver * GetTrackedDeviceDriver(uint32_t unWhich) override;
	virtual ITrackedDeviceServerDriver * FindTrackedDeviceDriver(const char * pchId) override;
	virtual void RunFrame() override;
	virtual bool ShouldBlockStandbyMode() override;
	virtual void EnterStandby() override;
	virtual void LeaveStandby() override;
public:
	IDriverLog *logger_;
	IServerDriverHost *driverHost_;
private:
	std::vector<CTrackedDevice *> trackedDevices_;
};

#endif // ServerDriver_H