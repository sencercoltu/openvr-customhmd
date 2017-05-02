#pragma once

#ifndef DirectStreamer_h
#define DirectStreamer_h

#include "Common.h"
#include "TCPServer.h"


#include <D3D11_1.h>
#include <DXGI1_2.h>


//#include <D3Dcompiler.h> //DirectMode disabled
//#include <DirectXMath.h> //DirectMode disabled



#include "AMDEnc\public\common\AMFFactory.h"
#include "AMDEnc\public\common\Thread.h"
#include "AMDEnc\public\common\InterfaceImpl.h"
#include "AMDEnc\public\include\components\VideoEncoderVCE.h"

#ifdef _DEBUG
#include "AMDEnc\public\common\TraceAdapter.h"
#endif //_DEBUG

using namespace vr;

//using namespace DirectX; //DirectMode disabled

struct InfoPacket {
	char H;
	char M;
	char D1;
	char D2;
	int Width;
	int Height;
};

//DirectMode disabled
/*
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

struct Vertex   
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
*/

struct DirectModeStreamer;

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
		m_TexIndex = -1;
		m_FrameReady = false;
		m_FileDump = nullptr;

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

		//DirectMode disabled
		//No need to render both eyes on one surface anymore
		/*
		ZeroMemory(&m_TlLeft, sizeof(m_TlLeft));
		ZeroMemory(&m_TlRight, sizeof(m_TlRight));
		ZeroMemory(&Ep, sizeof(Ep));
		m_SyncTexture = 0;
		m_pSyncTexture = nullptr;
		m_hTextureMapLock = nullptr;
		m_hBufferLock = nullptr;
		m_pRTView = nullptr;
		m_pRTTex = nullptr;
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
		*/
		m_FrameTime = (1.0f / 30.0f) * 1000.0f;
	}

	DWORD m_LastFrameTime;
	DWORD m_LastPacketReceive;
	float m_FrameTime;

	amf_pts m_EndcodeElapsed;

	void Init(CTrackedHMD *pHmd);
	void Destroy();
	void TextureFromHandle(SharedTextureHandle_t handle);

	
	HANDLE m_hDisplayThread;
	int m_DisplayState;
	bool m_IsRunning;
	unsigned short m_RemoteSequence;	
	uint64_t m_FrameCount;
	bool m_FrameReady;
	D3D_FEATURE_LEVEL m_FeatureLevel;

	ID3D11DeviceContext *m_pContext;
	ID3D11Device *m_pDevice;

	//DirectMode disabled
	/*
	SharedTextureHandle_t m_SyncTexture;
	ID3D11Texture2D *m_pSyncTexture;

	IDXGIKeyedMutex *m_pTexSync;
	ID3D11Texture2D *m_pRTTex;
	ID3D11RenderTargetView *m_pRTView;
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

	HANDLE m_hTextureMapLock;
	HANDLE m_hBufferLock;

	std::vector<TextureSet*> m_TextureSets;
	std::map<SharedTextureHandle_t, TextureLink> m_TextureMap;
	TextureLink m_TlLeft;
	TextureLink m_TlRight;

	void CombineEyes();
	*/

	amf::AMFContextPtr m_pEncderContext;
	amf::AMFComponentPtr m_pEncoder;
	
	struct TextureCache
	{
		vr::SharedTextureHandle_t m_hVirtualTexture;
		ID3D11Texture2D *m_pVirtualTexture;
		amf::AMFSurfacePtr m_pSurfaceTex;
		IDXGIKeyedMutex *m_pTexSync;
	};
	int m_TexIndex;

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
	static void TcpPacketReceive(void *dst, const char *pData, int len);
	void ProcessRemotePacket(USBPacket *pPacket);
	//int UpdateBuffer(DirectEyeData *pEyeData);
	

};

#endif //DirectStreamer_h