#ifndef TrackedHMD_H
#define TrackedHMD_H

#include "Common.h"
#include "TrackedDevice.h"

using namespace vr;

class CTrackedHMD : 
	public CTrackedDevice,
	public IVRDisplayComponent	
{
public:	
	~CTrackedHMD();

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

	CTrackedHMD(std::string id, CServerDriver *pServer);	
protected:
	virtual std::string GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
private:
	HANDLE m_hThread;
	bool m_IsRunning;
	unsigned int static WINAPI ProcessThread(void *p);
	void Run();
	HMDData m_HMDData;				
};

#endif // TrackedHMD_H