#ifndef TrackedDevice_H
#define TrackedDevice_H

#include "Common.h"

using namespace vr;
class CServerDriver;

class CTrackedDevice : 
	public ITrackedDeviceServerDriver, 
	public IVRDisplayComponent	
{
public:
	// Inherited via ITrackedDeviceServerDriver
	virtual EVRInitError Activate(uint32_t unObjectId) override;
	virtual void Deactivate() override;
	virtual void *GetComponent(const char *pchComponentNameAndVersion) override;
	virtual void DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) override;
	virtual void GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	virtual bool IsDisplayOnDesktop() override;
	virtual bool IsDisplayRealDisplay() override;
	virtual void GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight) override;
	virtual void GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	virtual void GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) override;
	virtual DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) override;
	//virtual TrackedDeviceDriverInfo_t GetTrackedDeviceDriverInfo() override;
	//virtual const char * GetModelNumber() override;
	//virtual const char * GetSerialNumber() override;
	virtual DriverPose_t GetPose() override;	
	virtual bool GetBoolTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	virtual float GetFloatTrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	virtual int32_t GetInt32TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	virtual uint64_t GetUint64TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	virtual HmdMatrix34_t GetMatrix34TrackedDeviceProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	virtual uint32_t GetStringTrackedDeviceProperty(ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, ETrackedPropertyError * pError) override;

	virtual void CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, void *(*pSharedTextureHandles)[2]);
	virtual void DestroySwapTextureSet(void *pSharedTextureHandle);
	virtual void DestroyAllSwapTextureSets(uint32_t unPid);
	virtual void SubmitLayer(void *pSharedTextureHandles[2], const vr::VRTextureBounds_t * pBounds, const vr::HmdMatrix34_t * pPose);
	virtual void Present();

	virtual void PowerOff();

public:
	struct HTData
	{
		int start;
		int led;
		int yaw;
		int pitch;
		int roll;
	};

	std::string m_Id;
	CTrackedDevice(std::string id, CServerDriver *pServer);

private:
	std::string GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError);
	IDriverLog *m_pLog;
	IServerDriverHost *m_pDriverHost;
	HANDLE m_hThread;
	bool m_IsRunning;
	unsigned int static WINAPI ProcessThread(void *p);
	void Run();
	MonitorData m_MonData;	
	float m_PIDValue;
	DriverPose_t m_Pose;

	uint32_t m_unObjectId;
};

#endif // TrackedDevice_H