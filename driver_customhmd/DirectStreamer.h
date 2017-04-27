#pragma once

#ifndef DirectStreamer_h
#define DirectStreamer_h

#include "Common.h"
#include "TCPServer.h"

extern "C" {
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavutil\avutil.h>
#include <libavutil\opt.h>
#include <libavutil\time.h>
#include <libswscale\swscale.h>
#include <libavutil\imgutils.h>
#include <libavformat\avio.h>
}

//#include <Evr.h>
#include <D3D11_1.h>
#include <DXGI1_2.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>



#define MESSAGE_MAX_BYTES 8192
#define RING_BUFFER_BYTES (1024 * 256 + MESSAGE_MAX_BYTES)
#define STREAM_PIX_FMT	AV_PIX_FMT_YUV420P 
#define VIDEO_CODEC_ID	AV_CODEC_ID_H264

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
	//ID3D11Resource *pGPUResource;

	//ID3D11Texture2D *pCPUTexture;
	//ID3D11Resource *pCPUResource;

	//IDXGISurface *pGPUSurface;
	//IMFSample *pVideoSample;

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
	XMMATRIX tTrans;
};

struct DirectEyeData
{
	TextureData *pData;
	D3D11_TEXTURE2D_DESC Desc;
	EVREye Eye;
};

typedef struct AVContext {
	AVFormatContext *fmt_ctx;
	AVCodecParameters *video_codecpar;
	AVCodecContext * video_dec_ctx;
	AVStream *video_stream;
	int video_stream_idx;
	AVFrame *frame;
	AVPacket pkt;
	enum AVMediaType type;
	AVDictionary *opts;
	AVCodec *dec;
} AVContext;

class DirectStreamer
{
public:
	DWORD LastFrameTime;
	float FrameTime;

	unsigned long BufferSize;
	unsigned char *pPixelBuffer[3];

	int FPS = 50;

	DWORD LastPacketReceive;

	DirectEyeData Left;
	DirectEyeData Right;

	USBRotationData RotData = {};

	AVCodec *pCodec;
	AVCodecContext *pCodecContext;
	AVFrame *pFrame;
	AVPacket *pPacket;

	int Width;
	int Height;

	int FrameCount;

private:
	int in_linesize[3];
	SwsContext *pSwsContext = nullptr;

	void SaveFrameYUV(AVFrame *pFrame, int frameno);
	void SaveFrameRGB(uint8_t *pixels, int size, int frameno);
public:
	int ProcessFrame(int pitch);
	

	AVPacket *GetPacket();

	bool Init(int width, int height, float fps, char *url);

	void Destroy();

	//bool InitMFT(int width, int height, float fps);

};

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

struct DirectModeData
{
	friend class CTrackedHMD;
	CTrackedHMD *pHMD;
public:

	DirectModeData()
	{
		ZeroMemory(&Ep, sizeof(Ep));
		ZeroMemory(&m_DirectScreen, sizeof(m_DirectScreen));
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
		m_pSquareIndexBuffer = nullptr;
		m_pSquareVertBuffer = nullptr;
		m_pVS = nullptr;
		m_pPS = nullptr;
		m_pVertLayout = nullptr;
		m_pVS_Buffer = nullptr;
		m_pPS_Buffer = nullptr;
		m_pSamplerState = nullptr;
		m_pConstantBuffer = nullptr;
		m_pCWcullMode = nullptr;;

	}

	void Init(CTrackedHMD *pHmd);
	void Destroy();

	HANDLE m_hDisplayThread;
	int m_DisplayState;
	bool m_IsRunning;
	unsigned short RemoteSequence;
	DirectStreamer m_DirectScreen;
	int m_FrameCount;

	SharedTextureHandle_t m_SyncTexture;
	ID3D11Texture2D *m_pSyncTexture;
	ID3D11DeviceContext *m_pContext;
	ID3D11Device *m_pDevice;

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


	D3D_FEATURE_LEVEL m_FeatureLevel;
	HANDLE m_hTextureMapLock;
	HANDLE m_hBufferLock;

	std::vector<TextureSet*> m_TextureSets;
	std::map<SharedTextureHandle_t, TextureLink> m_TextureMap;
	TextureLink m_TlLeft;
	TextureLink m_TlRight;

	void CombineEyes();
	unsigned int static WINAPI RemoteDisplayThread(void *p);
	void RunRemoteDisplay();
	static void TcpPacketReceive(void *dst, const char *pData, int len);
	void ProcessRemotePacket(USBPacket *pPacket);
	int UpdateBuffer(DirectEyeData *pEyeData);

};

#endif //DirectStreamer_h