#ifndef TrackedHMD_H
#define TrackedHMD_H

#include "Common.h"
#include "TrackedDevice.h"
#include "hidapi.h"
#include "SensorFusion.h"

using namespace vr;

class CTrackedHMD : 
	public CTrackedDevice,
	public IVRDisplayComponent //, public IVRCameraComponent
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

	virtual void CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, void *(*pSharedTextureHandles)[3]);
	virtual void DestroySwapTextureSet(void *pSharedTextureHandle);
	virtual void DestroyAllSwapTextureSets(uint32_t unPid);
	virtual void GetNextSwapTextureSetIndex(void *pSharedTextureHandles[2], uint32_t(*pIndices)[2]);
	virtual void SubmitLayer(void *pSharedTextureHandles[2], const vr::VRTextureBounds_t(&bounds)[2], const vr::HmdMatrix34_t *pPose);
	virtual void Present(void *hSyncTexture);

	virtual void PowerOff();
	virtual void RunFrame();
public:
/*	struct HTData
	{
		int start; 
		int led;
		int yaw;
		int pitch;
		int roll;
	};
*/
	struct QOrient
	{
		double w;
		double x;
		double y;
		double z;
	};
	CTrackedHMD(std::string id, CServerDriver *pServer);	
protected:
	virtual std::string GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
private:
	CSensorFusion m_SensorFusion;
	HANDLE m_hThread;
	bool m_IsRunning;
	unsigned int static WINAPI ProcessThread(void *p);
	void Run();
	HMDData m_HMDData;				
	void OpenUSB(hid_device **ppHandle);
	void CloseUSB(hid_device **ppHandle);
	/*
	// Inherited via IVRCameraComponent
	virtual bool HasCamera() override;
	virtual bool GetCameraFirmwareDescription(char * pBuffer, uint32_t nBufferLen) override;
	virtual bool GetCameraFrameDimensions(vr::ECameraVideoStreamFormat nVideoStreamFormat, uint32_t * pWidth, uint32_t * pHeight) override;
	virtual bool GetCameraFrameBufferingRequirements(int * pDefaultFrameQueueSize, uint32_t * pFrameBufferDataSize) override;
	virtual bool SetCameraFrameBuffering(int nFrameBufferCount, void ** ppFrameBuffers, uint32_t nFrameBufferDataSize) override;
	virtual bool SetCameraVideoStreamFormat(vr::ECameraVideoStreamFormat nVideoStreamFormat) override;
	virtual vr::ECameraVideoStreamFormat GetCameraVideoStreamFormat() override;
	virtual bool StartVideoStream() override;
	virtual void StopVideoStream() override;
	virtual bool IsVideoStreamActive() override;
	virtual float GetVideoStreamElapsedTime() override;
	virtual const vr::CameraVideoStreamFrame_t * GetVideoStreamFrame() override;
	virtual void ReleaseVideoStreamFrame(const vr::CameraVideoStreamFrame_t * pFrameImage) override;
	virtual bool SetAutoExposure(bool bEnable) override;
	virtual bool PauseVideoStream() override;
	virtual bool ResumeVideoStream() override;
	virtual bool IsVideoStreamPaused() override;
	virtual bool GetCameraDistortion(float flInputU, float flInputV, float * pflOutputU, float * pflOutputV) override;
	virtual bool GetCameraProjection(float flWidthPixels, float flHeightPixels, float flZNear, float flZFar, vr::HmdMatrix44_t * pProjection) override;
	virtual bool GetRecommendedCameraUndistortion(uint32_t * pUndistortionWidthPixels, uint32_t * pUndistortionHeightPixels) override;
	virtual bool SetCameraUndistortion(uint32_t nUndistortionWidthPixels, uint32_t nUndistortionHeightPixels) override;
	virtual bool GetCameraFirmwareVersion(uint64_t * pFirmwareVersion) override;
	virtual bool SetFrameRate(int nISPFrameRate, int nSensorFrameRate) override;
	virtual bool SetCameraVideoSinkCallback(vr::ICameraVideoSinkCallback * pCameraVideoSinkCallback) override;
	virtual bool GetCameraCompatibilityMode(vr::ECameraCompatibilityMode * pCameraCompatibilityMode) override;
	virtual bool SetCameraCompatibilityMode(vr::ECameraCompatibilityMode nCameraCompatibilityMode) override;
	*/
};

#endif // TrackedHMD_H