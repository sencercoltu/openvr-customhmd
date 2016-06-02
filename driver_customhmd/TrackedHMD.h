#ifndef TrackedHMD_H
#define TrackedHMD_H

#include "TrackedDevice.h"

using namespace vr;

class CTrackedHMD : 
	public CTrackedDevice,
	public IVRDisplayComponent , public IVRCameraComponent
{
private:	
	HMDData m_HMDData;			
	//unsigned int static WINAPI CameraThread(void *p);
	//void RunCamera();

public:
	CTrackedHMD(std::string displayName, CServerDriver *pServer);
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

	// Inherited via IVRCameraComponent
	bool HasCamera() override;
	bool GetCameraFirmwareDescription(char *pBuffer, uint32_t nBufferLen) override;
	bool GetCameraFrameDimensions(vr::ECameraVideoStreamFormat nVideoStreamFormat, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraFrameBufferingRequirements(int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize) override;
	bool SetCameraFrameBuffering(int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize) override;
	bool SetCameraVideoStreamFormat(vr::ECameraVideoStreamFormat nVideoStreamFormat) override;
	vr::ECameraVideoStreamFormat GetCameraVideoStreamFormat() override;
	bool StartVideoStream() override;
	void StopVideoStream() override;
	bool IsVideoStreamActive() override;
	float GetVideoStreamElapsedTime() override;
	const vr::CameraVideoStreamFrame_t *GetVideoStreamFrame() override;
	void ReleaseVideoStreamFrame(const vr::CameraVideoStreamFrame_t *pFrameImage) override;
	bool SetAutoExposure(bool bEnable) override;
	bool PauseVideoStream() override;
	bool ResumeVideoStream() override;
	bool IsVideoStreamPaused() override;
	bool GetCameraDistortion(float flInputU, float flInputV, float *pflOutputU, float *pflOutputV) override;
	bool GetCameraProjection(float flWidthPixels, float flHeightPixels, float flZNear, float flZFar, vr::HmdMatrix44_t *pProjection) override;
	bool GetRecommendedCameraUndistortion(uint32_t *pUndistortionWidthPixels, uint32_t *pUndistortionHeightPixels) override;
	bool SetCameraUndistortion(uint32_t nUndistortionWidthPixels, uint32_t nUndistortionHeightPixels) override;
	bool GetCameraFirmwareVersion(uint64_t *pFirmwareVersion) override;
	bool SetFrameRate(int nISPFrameRate, int nSensorFrameRate) override;
	bool SetCameraVideoSinkCallback(vr::ICameraVideoSinkCallback *pCameraVideoSinkCallback) override;
	bool GetCameraCompatibilityMode(vr::ECameraCompatibilityMode *pCameraCompatibilityMode) override;
	bool SetCameraCompatibilityMode(vr::ECameraCompatibilityMode nCameraCompatibilityMode) override;
	bool GetCameraFrameBounds(vr::EVRTrackedCameraFrameType eFrameType, uint32_t *pLeft, uint32_t *pTop, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraIntrinsics(vr::EVRTrackedCameraFrameType eFrameType, HmdVector2_t *pFocalLength, HmdVector2_t *pCenter) override;


protected:
	std::string GetStringProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
	bool GetBoolProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError) override;
	float GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	int32_t GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	uint64_t GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	void PoseUpdate(USBData *pData, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos) override;
	void RunFrame(DWORD currTick) override;

private:	
	bool InitCamera();
	void DeinitCamera();
	void YUY2toNV12(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride);
	static void OnCameraFrameUpdateCallback(char *pFrame, int width, int height, int stride, GUID *pMediaFormat, void *pUserData);
	void OnCameraFrameUpdate(char *pFrame, int width, int height, int stride, GUID *pMediaFormat);
};

#endif // TrackedHMD_H