#ifndef TrackedHMD_H
#define TrackedHMD_H

#include "TrackedDevice.h"

using namespace vr;

class CTrackedHMD : 
	public CTrackedDevice,
	public IVRDisplayComponent //, public IVRCameraComponent
{
private:
	IVRSettings *m_pSettings;
	HMDData m_HMDData;		
public:
	CTrackedHMD(std::string id, CServerDriver *pServer);	
	~CTrackedHMD();
	EVRInitError Activate(uint32_t unObjectId) override;
	void Deactivate() override;
	void *GetComponent(const char *pchComponentNameAndVersion) override;
	void DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) override;
	void GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	bool IsDisplayOnDesktop() override;
	bool IsDisplayRealDisplay() override;
	void GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight) override;
	void GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	void GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) override;
	DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) override;
	DriverPose_t GetPose() override;
	
	void CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, void *(*pSharedTextureHandles)[3]) override;
	void DestroySwapTextureSet(void *pSharedTextureHandle) override;
	void DestroyAllSwapTextureSets(uint32_t unPid) override;
	void GetNextSwapTextureSetIndex(void *pSharedTextureHandles[2], uint32_t(*pIndices)[2]) override;
	void SubmitLayer(void *pSharedTextureHandles[2], const vr::VRTextureBounds_t(&bounds)[2], const vr::HmdMatrix34_t *pPose) override;
	void Present(void *hSyncTexture) override;
	void PowerOff() override;	

protected:
	std::string GetStringProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
	bool GetBoolProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
	float GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	int32_t GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	uint64_t GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	void PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler) override;
	void RunFrame(DWORD currTick) override;
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