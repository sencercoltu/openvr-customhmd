#pragma once

#ifndef VirtualStreamer_h
#define VirtualStreamer_h

#include "Common.h"
#include "TCPServer.h"


#include <D3D11_1.h>
#include <DXGI1_2.h>


#include "AMDEnc\public\common\AMFFactory.h"
#include "AMDEnc\public\common\Thread.h"
#include "AMDEnc\public\common\InterfaceImpl.h"
#include "AMDEnc\public\include\components\VideoEncoderVCE.h"

#ifdef _DEBUG
#include "AMDEnc\public\common\TraceAdapter.h"
#endif //_DEBUG

using namespace vr;

struct InfoPacket 
{
	char H;
	char M;
	char D1;
	char D2;
	int Width;
	int Height;
	int FrameRate;
};



struct VirtualStreamer;

struct VirtualStreamer : public amf::AMFSurfaceObserver
{
	friend class CTrackedHMD;	
public:

	virtual void AMF_STD_CALL OnSurfaceDataRelease(amf::AMFSurface* pSurface) override
	{

	}

	VirtualStreamer()
	{
		m_pServer = nullptr;

		m_LastCameraStatus = false;
		m_SequenceCounter = 0;
		m_pEncoderContext = nullptr;
		m_pEncoder = nullptr;
		m_TexIndex = -1;
		m_FrameReady = false;		

		//MFT disabled
		/*
		m_ManagerToken = 0;
		m_pManager = nullptr;
		m_pTexSync = nullptr;
		m_pTransform = nullptr;
		m_pOutputSample = nullptr;
		m_pInputSample = nullptr;
		m_pEventGenerator = nullptr;
		*/

		ZeroMemory(&m_TextureCache, sizeof(m_TextureCache));

		//m_pVirtualTexture = nullptr;
		m_hDisplayThread = nullptr;
		m_IsRunning = false;
		m_DisplayState = 0;
		m_RemoteSequence = 0;
		m_FrameCount = 0;
		m_FeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;

		m_pContext = nullptr;
		m_pDevice = nullptr;
		
		m_EndcodeElapsed = 0;
		m_FrameTime = (1.0f / 30.0f) * 1000.0f;
	}

	CTrackedHMD *pHMD;

	amf_pts m_LastFrameTime;
	DWORD m_LastPacketReceive;
	float m_FrameTime;

	amf_pts m_EndcodeElapsed;

	void Init(CTrackedHMD *pHmd);
	void Destroy();
	void TextureFromHandle(SharedTextureHandle_t handlei);

	CTCPServer *m_pServer;

	HANDLE m_hDisplayThread;
	int m_DisplayState;
	bool m_IsRunning;
	unsigned short m_RemoteSequence;	
	uint64_t m_FrameCount;
	bool m_FrameReady;
	D3D_FEATURE_LEVEL m_FeatureLevel;

	ID3D11DeviceContext *m_pContext;
	ID3D11Device *m_pDevice;

	amf::AMFContextPtr m_pEncoderContext;
	amf::AMFComponentPtr m_pEncoder;
	
	struct TextureCache
	{
		vr::SharedTextureHandle_t m_hVirtualTexture;
		ID3D11Texture2D *m_pVirtualTexture;
		amf::AMFSurfacePtr m_pSurfaceTex;
		IDXGIKeyedMutex *m_pTexSync;
		uint64_t m_Sequence;
	};
	int m_TexIndex;	
	uint64_t m_SequenceCounter;

	TextureCache m_TextureCache[3];

	bool InitEncoder();
	void DestroyEncoder();

	//MFT disabled
	/*
	//IMFMediaBuffer *m_pMediaBuffer;
	UINT m_ManagerToken;
	IMFDXGIDeviceManager *m_pManager;
	IMFTransform *m_pTransform;
	IMFMediaEventGenerator *m_pEventGenerator;
	IMFSample *m_pInputSample;
	IMFSample *m_pOutputSample;

	bool InitMFT();
	void DestroyMFT();
	IMFMediaEvent *GetEvent();
	void ProcessEvent(IMFMediaEvent *pEvent, CTCPServer *pServer);
	*/

	unsigned int static WINAPI RemoteDisplayThread(void *p);
	void RunRemoteDisplay();
	static void TcpPacketReceive(void *dst, VirtualPacketTypes type, const char *pData, int len);
	void ProcessRemotePacket(USBPacket *pPacket);
	//int UpdateBuffer(DirectEyeData *pEyeData);
	
	bool m_LastCameraStatus;
	void EnableCamera(bool enable);

};

#endif //VirtualStreamer_h