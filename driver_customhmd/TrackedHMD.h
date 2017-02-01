#ifndef TrackedHMD_H
#define TrackedHMD_H

#include "TrackedDevice.h" 

using namespace vr;

class CTrackedHMD : 
	public IVRDisplayComponent,
	public IVRCameraComponent,
	public CTrackedDevice
{
private:	
	HMDData m_HMDData;		
	CameraData m_Camera;

	//unsigned int static WINAPI CameraThread(void *p);
	//void RunCamera();

public:
	CTrackedHMD(std::string displayName, CServerDriver *pServer);
	~CTrackedHMD();
	bool IsConnected();
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
	//void PowerOff() override;

	/*
	void CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, void *(*pSharedTextureHandles)[3]) override;
	void DestroySwapTextureSet(void *pSharedTextureHandle) override;
	void DestroyAllSwapTextureSets(uint32_t unPid) override;
	void GetNextSwapTextureSetIndex(void *pSharedTextureHandles[2], uint32_t(*pIndices)[2]) override;
	void SubmitLayer(void *pSharedTextureHandles[2], const VRTextureBounds_t(&bounds)[2], const HmdMatrix34_t *pPose) override;
	void Present(void *hSyncTexture) override;
	*/

	// Inherited via IVRCameraComponent
	//bool HasCamera() override;
	//bool GetCameraFirmwareDescription(char *pBuffer, uint32_t nBufferLen) override;
	bool GetCameraFrameDimensions(ECameraVideoStreamFormat nVideoStreamFormat, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraFrameBufferingRequirements(int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize) override;
	bool SetCameraFrameBuffering(int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize) override;
	bool SetCameraVideoStreamFormat(ECameraVideoStreamFormat nVideoStreamFormat) override;
	ECameraVideoStreamFormat GetCameraVideoStreamFormat() override;
	bool StartVideoStream() override;
	void StopVideoStream() override;
	//bool IsVideoStreamActive() override;
	//float GetVideoStreamElapsedTime() override;
	const CameraVideoStreamFrame_t *GetVideoStreamFrame() override;
	void ReleaseVideoStreamFrame(const CameraVideoStreamFrame_t *pFrameImage) override;
	bool SetAutoExposure(bool bEnable) override;
	bool PauseVideoStream() override;
	bool ResumeVideoStream() override;
	bool IsVideoStreamActive(bool *pbPaused, float *pflElapsedTime) override;
	//bool IsVideoStreamPaused() override;
	bool GetCameraDistortion(float flInputU, float flInputV, float *pflOutputU, float *pflOutputV) override;
	bool GetCameraProjection(vr::EVRTrackedCameraFrameType eFrameType, float flZNear, float flZFar, vr::HmdMatrix44_t *pProjection) override;
	//bool GetCameraProjection(float flWidthPixels, float flHeightPixels, float flZNear, float flZFar, HmdMatrix44_t *pProjection) override;
	//bool GetRecommendedCameraUndistortion(uint32_t *pUndistortionWidthPixels, uint32_t *pUndistortionHeightPixels) override;
	//bool SetCameraUndistortion(uint32_t nUndistortionWidthPixels, uint32_t nUndistortionHeightPixels) override;
	//bool GetCameraFirmwareVersion(uint64_t *pFirmwareVersion) override;
	bool SetFrameRate(int nISPFrameRate, int nSensorFrameRate) override;
	bool SetCameraVideoSinkCallback(ICameraVideoSinkCallback *pCameraVideoSinkCallback) override;
	bool GetCameraCompatibilityMode(ECameraCompatibilityMode *pCameraCompatibilityMode) override;
	bool SetCameraCompatibilityMode(ECameraCompatibilityMode nCameraCompatibilityMode) override;
	bool GetCameraFrameBounds(EVRTrackedCameraFrameType eFrameType, uint32_t *pLeft, uint32_t *pTop, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraIntrinsics(EVRTrackedCameraFrameType eFrameType, HmdVector2_t *pFocalLength, HmdVector2_t *pCenter) override;


protected:
	//std::string GetStringProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError) override;
	//bool GetBoolProperty(ETrackedDeviceProperty prop, ETrackedPropertyError *pError) override;
	//float GetFloatProperty(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	//int32_t GetInt32Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	//uint64_t GetUint64Property(ETrackedDeviceProperty prop, ETrackedPropertyError * pError) override;
	void PacketReceived(USBPacket *pPacket, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos) override;
	void RunFrame(DWORD currTick) override;

private:		
	void SetupCamera();
	static void CameraFrameUpdateCallback(CCaptureDevice *pCaptureDevice, void *pUserData);
	void OnCameraFrameUpdate();	
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

protected:
	void SetDefaultProperties() override;
	bool ReportsTimeSinceVSync;
	float SecondsFromVsyncToPhotons;
	float DisplayFrequency;
	float UserIpdMeters;
	uint64_t CurrentUniverseId;
	uint64_t PreviousUniverseId;
	uint64_t DisplayFirmwareVersion;
	bool IsOnDesktop;
	int32_t DisplayMCType;
	float DisplayMCOffset;
	float DisplayMCScale;
	int32_t EdidVendorID;
	std::string DisplayMCImageLeft;
	std::string DisplayMCImageRight;
	float DisplayGCBlackClamp;
	int32_t EdidProductID;
	HmdMatrix34_t CameraToHeadTransform;
	int32_t DisplayGCType;
	float DisplayGCOffset;
	float DisplayGCScale;
	float DisplayGCPrescale;
	std::string DisplayGCImage;
	float LensCenterLeftU;
	float LensCenterLeftV;
	float LensCenterRightU;
	float LensCenterRightV;
	float UserHeadToEyeDepthMeters;
	uint64_t CameraFirmwareVersion;
	std::string CameraFirmwareDescription;
	uint64_t DisplayFPGAVersion;
	uint64_t DisplayBootloaderVersion;
	uint64_t DisplayHardwareVersion;
	uint64_t AudioFirmwareVersion;
	int32_t CameraCompatibilityMode;
	float ScreenshotHorizontalFieldOfViewDegrees;
	float ScreenshotVerticalFieldOfViewDegrees;
	bool DisplaySuppressed;
	bool DisplayAllowNightMode;
	int32_t DisplayMCImageWidth;
	int32_t DisplayMCImageHeight;
	int32_t DisplayMCImageNumChannels;
	void *DisplayMCImageData;
	bool UsesDriverDirectMode;
};

#endif // TrackedHMD_H