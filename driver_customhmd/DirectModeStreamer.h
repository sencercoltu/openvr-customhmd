#pragma once

#ifndef DirectStreamer_h
#define DirectStreamer_h

#include "Common.h"
#include "TCPServer.h"


//#include <D2d1.h>
#include <D3D11_1.h>
#include <DXGI1_2.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
//#include <Codecapi.h>
//#include <Evr.h>


#include "AMDEnc\public\common\AMFFactory.h"
#include "AMDEnc\public\common\Thread.h"
#include "AMDEnc\public\common\InterfaceImpl.h"
#include "AMDEnc\public\include\components\VideoEncoderVCE.h"

#ifdef _DEBUG
#include "AMDEnc\public\common\TraceAdapter.h"
#endif //_DEBUG

using namespace vr;
using namespace DirectX;

struct InfoPacket {
	char H;
	char M;
	char D1;
	char D2;
	int Width;
	int Height;
};

struct TextureData
{
	uint32_t Index;
	EVREye Eye;

	ID3D11Texture2D *pGPUTexture;
	ID3D11ShaderResourceView *pShaderResourceView;
	HANDLE hSharedHandle;
};

struct TextureSet
{
	uint32_t Pid;
	TextureData Data[3];
	bool HasHandle(HANDLE handle)
	{
		for (auto i = 0; i < 3; i++)
		{
			if (Data[i].hSharedHandle == handle)
				return true;
		}
		return false;
	}
};

struct TextureLink
{
	TextureData *pData;
	TextureSet *pSet;	
};

struct DirectEyeData
{
	TextureData *pData;
	D3D11_TEXTURE2D_DESC Desc;
	EVREye Eye;
};

//struct DirectStreamer
//{
//public:
//
//
//	unsigned long BufferSize;
//	unsigned char *pPixelBuffer[3];
//
//	int FPS = 50;
//
//	DWORD LastPacketReceive;
//
//	DirectEyeData Left;
//	DirectEyeData Right;
//
//	USBRotationData RotData = {};
//
//	int Width;
//	int Height;
//	int FrameCount;
//
//private:	
//
//public:
//	bool Init(CTrackedHMD *pHmd);
//
//	IMFMediaEvent *GetEvent();
//
//	void Destroy();
//
//};

//Vertex Structure and Vertex Layout (Input Layout)//

struct Vertex    //Overloaded Vertex Structure
{
	Vertex() {}
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z),  tex(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};


struct CbEyePos
{
	CbEyePos() {}
	XMMATRIX SHIFT;
};


struct DirectModeStreamer;

//class CMFTEventReceiver : 
//	public IMFAsyncCallback
//{
//private:
//	ULONG _refCount;
//	DirectModeStreamer *m_pStreamer;
//public:
//	CMFTEventReceiver(DirectModeStreamer *pStreamer)
//	{
//		m_pStreamer = pStreamer;
//		_refCount = 1;			
//	}
//	virtual HRESULT STDMETHODCALLTYPE GetParameters( /* [out] */ __RPC__out DWORD *pdwFlags, /* [out] */ __RPC__out DWORD *pdwQueue) override;
//	virtual HRESULT STDMETHODCALLTYPE Invoke( /* [in] */ __RPC__in_opt IMFAsyncResult *pAsyncResult) override;
//	
//	virtual HRESULT QueryInterface(REFIID riid, void ** ppvObject) override;
//
//	virtual ULONG AddRef(void) override;
//
//	virtual ULONG Release(void) override;
//};

struct DirectModeStreamer : public amf::AMFSurfaceObserver
{
	friend class CTrackedHMD;
	CTrackedHMD *pHMD;
public:

	virtual void AMF_STD_CALL OnSurfaceDataRelease(amf::AMFSurface* pSurface) override
	{

	}

	FILE *m_FileDump;

	DirectModeStreamer()
	{
		m_pEncderContext = nullptr;
		m_pEncoder = nullptr;
		m_pSurfaceTex = nullptr;
		m_pSurfaceFrame = nullptr;

		m_FrameReady = false;

		m_FileDump = nullptr;
		//m_pMediaBuffer = nullptr;
		m_pTexBuffer = nullptr;

		/*
		m_ManagerToken = 0;
		m_pManager = nullptr;
		m_pTexSync = nullptr;
		m_pTransform = nullptr;
		m_pOutputSample = nullptr;
		m_pInputSample = nullptr;
		m_pEventGenerator = nullptr;
		*/

		ZeroMemory(&Ep, sizeof(Ep));
		//ZeroMemory(&m_Streamer, sizeof(m_Streamer));
		m_hDisplayThread = nullptr;
		m_IsRunning = false;
		m_DisplayState = 0;
		RemoteSequence = 0;		
		m_FrameCount = 0;
		m_SyncTexture = 0;
		m_pSyncTexture = nullptr;
		m_pContext = nullptr;
		m_pDevice = nullptr;
		m_FeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;
		m_hTextureMapLock = nullptr;
		m_hBufferLock = nullptr;

		ZeroMemory(&m_TlLeft, sizeof(m_TlLeft));
		ZeroMemory(&m_TlRight, sizeof(m_TlRight));

		m_pRTView = nullptr;
		m_pRTTex = nullptr;
		m_pRTRes = nullptr;
		m_pSquareIndexBuffer = nullptr;
		m_pSquareVertBuffer = nullptr;
		m_pVS = nullptr;
		m_pPS = nullptr;
		m_pVertLayout = nullptr;
		m_pVS_Buffer = nullptr;
		m_pPS_Buffer = nullptr;
		m_pSamplerState = nullptr;
		m_pConstantBuffer = nullptr;
		m_pCWcullMode = nullptr;		


	}

	DWORD m_LastFrameTime, m_LastPacketReceive;
	float m_FrameTime;

	void Init(CTrackedHMD *pHmd);
	void Destroy();

	HANDLE m_hDisplayThread;
	int m_DisplayState;
	bool m_IsRunning;
	unsigned short RemoteSequence;
	//DirectStreamer m_Streamer;
	int m_FrameCount;
	bool m_FrameReady;

	SharedTextureHandle_t m_SyncTexture;
	ID3D11Texture2D *m_pSyncTexture;
	ID3D11DeviceContext *m_pContext;
	ID3D11Device *m_pDevice;

	IDXGIKeyedMutex *m_pTexSync;
	ID3D11Texture2D *m_pRTTex;
	ID3D11Resource *m_pTexBuffer;
	ID3D11RenderTargetView *m_pRTView;
	ID3D11Resource *m_pRTRes;
	ID3D11SamplerState *m_pSamplerState;
	ID3D11Buffer *m_pConstantBuffer;
	CbEyePos Ep;

	ID3D11Buffer *m_pSquareIndexBuffer;
	ID3D11Buffer *m_pSquareVertBuffer;
	ID3D11VertexShader* m_pVS;
	ID3D11PixelShader* m_pPS;
	ID3DBlob* m_pVS_Buffer;
	ID3DBlob* m_pPS_Buffer;
	ID3D11InputLayout* m_pVertLayout;
	ID3D11RasterizerState* m_pCWcullMode;


	D3D_FEATURE_LEVEL m_FeatureLevel;
	HANDLE m_hTextureMapLock;
	HANDLE m_hBufferLock;

	std::vector<TextureSet*> m_TextureSets;
	std::map<SharedTextureHandle_t, TextureLink> m_TextureMap;
	TextureLink m_TlLeft;
	TextureLink m_TlRight;


	amf::AMFContextPtr m_pEncderContext;
	amf::AMFComponentPtr m_pEncoder;
	amf::AMFSurfacePtr m_pSurfaceTex;
	amf::AMFSurfacePtr m_pSurfaceFrame;

	bool InitEncoder();
	void DestroyEncoder();

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

	void CombineEyes();
	unsigned int static WINAPI RemoteDisplayThread(void *p);
	void RunRemoteDisplay();
	static void TcpPacketReceive(void *dst, const char *pData, int len);
	void ProcessRemotePacket(USBPacket *pPacket);
	//int UpdateBuffer(DirectEyeData *pEyeData);
	

};

#endif //DirectStreamer_h