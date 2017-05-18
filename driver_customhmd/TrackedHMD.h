#pragma once

#ifndef TrackedHMD_H
#define TrackedHMD_H

#include "TrackedDevice.h" 
#include "VirtualStreamer.h" 
#include "DirectModeOutput.h" 

using namespace vr;

enum DisplayMode
{
	SteamDirect,
	SteamExtended,
	DirectMode,
	Virtual
};

class CDriverDirectModeComponentFix;
class CDriverVirtualDisplayComponentFix;

class CTrackedHMD : 
	public IVRDisplayComponent, 
	public IVRCameraComponent,
	public CTrackedDevice
{
	friend struct VirtualStreamer;
	friend struct DirectModeOutput;

private:	
	HMDData m_HMDData;		
	CameraData m_Camera;
	VirtualStreamer m_VirtualDisplay;
	DirectModeOutput m_DirectOutput;
	DisplayMode m_DisplayMode;
	
	CDriverVirtualDisplayComponentFix *m_pVDF;
	CDriverDirectModeComponentFix *m_pDMF;

	void ReloadDistortionMap();

	class CDistortionMap
	{
	private:
		int Width;
		int Height;
		unsigned char *pData;
	public:
		CDistortionMap(int width, int height, unsigned char *data)
		{
			Height = height;
			Width = width;			
			pData = data;
		}
		~CDistortionMap()
		{
			if (pData)
				free(pData);
			pData = nullptr;
			Width = Height = 0;
		}
		void FillUV(EVREye eEye, float fU, float fV, DistortionCoordinates_t *pCoords)
		{
			//TODO: mirror for right eye

			if (!pData)
				return;

			int x = (int)((Width - 1) * ((eEye == EVREye::Eye_Right)? 1.0f - fU : fU));
			int y = (int)((Height - 1) * (1.0f - fV));
			
			//use same map for all channels for now

			int pos = ((Width * y) + x) * 3;
			float u = ((float)pData[pos++]) / 255.0f;
			float v = ((float)pData[pos++]) / 255.0f;
			//float b = ((float)pData[pos]) / 256.0f;

			//if (fU < 0.5) u = 0; else u = 1;
			//if (fV < 0.5) v = 0; else v = 1;

			pCoords->rfRed[0] = fU * u;
			pCoords->rfRed[1] = fV * v;

			pCoords->rfGreen[0] = fU * u;
			pCoords->rfGreen[1] = fV * v;

			pCoords->rfBlue[0] = fU * u;
			pCoords->rfBlue[1] = fV * v;
		}
	};

	CDistortionMap *m_pDistortionMap;

public:
	CTrackedHMD(std::string displayName, CServerDriver *pServer);
	~CTrackedHMD();
	bool IsConnected();
	DriverPose_t GetPose() override;

public: //ITrackedDeviceServerDriver
	EVRInitError Activate(uint32_t unObjectId) override;
	void Deactivate() override;
	void EnterStandby() override;
	void *GetComponent(const char *pchComponentNameAndVersion) override;
	void DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) override;

public: //IVRDisplayComponent
	void GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	bool IsDisplayOnDesktop() override;
	bool IsDisplayRealDisplay() override;
	void GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight) override;
	void GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	void GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) override;
	DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) override;

public: //IVRCameraComponent
	bool GetCameraFrameDimensions(ECameraVideoStreamFormat nVideoStreamFormat, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraFrameBufferingRequirements(int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize) override;
	bool SetCameraFrameBuffering(int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize) override;
	bool SetCameraVideoStreamFormat(ECameraVideoStreamFormat nVideoStreamFormat) override;
	ECameraVideoStreamFormat GetCameraVideoStreamFormat() override;
	bool StartVideoStream() override;
	void StopVideoStream() override;
	const CameraVideoStreamFrame_t *GetVideoStreamFrame() override;
	void ReleaseVideoStreamFrame(const CameraVideoStreamFrame_t *pFrameImage) override;
	bool SetAutoExposure(bool bEnable) override;
	bool PauseVideoStream() override;
	bool ResumeVideoStream() override;
	bool IsVideoStreamActive(bool *pbPaused, float *pflElapsedTime) override;
	bool GetCameraDistortion(float flInputU, float flInputV, float *pflOutputU, float *pflOutputV) override;
	bool GetCameraProjection(vr::EVRTrackedCameraFrameType eFrameType, float flZNear, float flZFar, vr::HmdMatrix44_t *pProjection) override;
	bool SetFrameRate(int nISPFrameRate, int nSensorFrameRate) override;
	bool SetCameraVideoSinkCallback(ICameraVideoSinkCallback *pCameraVideoSinkCallback) override;
	bool GetCameraCompatibilityMode(ECameraCompatibilityMode *pCameraCompatibilityMode) override;
	bool SetCameraCompatibilityMode(ECameraCompatibilityMode nCameraCompatibilityMode) override;
	bool GetCameraFrameBounds(EVRTrackedCameraFrameType eFrameType, uint32_t *pLeft, uint32_t *pTop, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraIntrinsics(EVRTrackedCameraFrameType eFrameType, HmdVector2_t *pFocalLength, HmdVector2_t *pCenter) override;


public: //IVRDriverDirectModeComponent
	void CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3]);
	void DestroySwapTextureSet(vr::SharedTextureHandle_t sharedTextureHandle);
	void DestroyAllSwapTextureSets(uint32_t unPid);
	void GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2]);
	void SubmitLayer(vr::SharedTextureHandle_t sharedTextureHandles[2], const vr::VRTextureBounds_t(&bounds)[2], const vr::HmdMatrix34_t *pPose);
	void DirectPresent(vr::SharedTextureHandle_t syncTexture);

public:  //IVRVirtualDisplay 
	void VirtualPresent(vr::SharedTextureHandle_t backbufferTextureHandle);
	void WaitForPresent();
	bool GetTimeSinceLastVsync(float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter);

protected: //CTrackedDevice
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
};

class CDriverDirectModeComponentFix : public IVRDriverDirectModeComponent
{
private:
	CTrackedHMD *m_pHMD;
public:
	CDriverDirectModeComponentFix(CTrackedHMD *pHmd) { m_pHMD = pHmd; }
	void CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3]) override { m_pHMD->CreateSwapTextureSet(unPid, unFormat, unWidth, unHeight, pSharedTextureHandles); }
	void DestroySwapTextureSet(vr::SharedTextureHandle_t sharedTextureHandle) override { m_pHMD->DestroySwapTextureSet(sharedTextureHandle); }
	void DestroyAllSwapTextureSets(uint32_t unPid) override { m_pHMD->DestroyAllSwapTextureSets(unPid); }
	void GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2]) override { m_pHMD->GetNextSwapTextureSetIndex(sharedTextureHandles, pIndices); }
	void SubmitLayer(vr::SharedTextureHandle_t sharedTextureHandles[2], const vr::VRTextureBounds_t(&bounds)[2], const vr::HmdMatrix34_t *pPose) override { m_pHMD->SubmitLayer(sharedTextureHandles, bounds, pPose); }
	void Present(vr::SharedTextureHandle_t syncTexture) override { m_pHMD->DirectPresent(syncTexture); }
};

class CDriverVirtualDisplayComponentFix : public IVRVirtualDisplay
{
private:
	CTrackedHMD *m_pHMD;
public:
	CDriverVirtualDisplayComponentFix(CTrackedHMD *pHmd) { m_pHMD = pHmd; }
	void WaitForPresent() override { m_pHMD->WaitForPresent(); }
	bool GetTimeSinceLastVsync(float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter) override { return m_pHMD->GetTimeSinceLastVsync(pfSecondsSinceLastVsync, pulFrameCounter); }
	void Present(vr::SharedTextureHandle_t backbufferTextureHandle) override { m_pHMD->VirtualPresent(backbufferTextureHandle); }
};


#endif // TrackedHMD_H