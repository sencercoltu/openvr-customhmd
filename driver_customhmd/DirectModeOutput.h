#pragma once

#ifndef DirectModeOutput_h
#define DirectModeOutput_h

#include "Common.h"

#include <D3D11_1.h>
#include <DXGI1_2.h>
#include <D3Dcompiler.h> 
#include <DirectXMath.h> 

using namespace DirectX; 


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
	VRTextureBounds_t Bounds;
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
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), tex(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};


struct CbEyePos
{
	CbEyePos() {}
	XMMATRIX SHIFT;
	float ROTATE;
};


struct DirectModeOutput
{
public:
	DirectModeOutput()
	{		
		m_IsRunning = false;
		m_hWnd = nullptr;
		m_ZAxis = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		m_pContext = nullptr;
		m_pDevice = nullptr;

		m_LeftRotate = 0;
		m_RightRotate = 0; 

		ZeroMemory(&m_TlLeft, sizeof(m_TlLeft));
		ZeroMemory(&m_TlRight, sizeof(m_TlRight));
		ZeroMemory(&Ep, sizeof(Ep));
		m_SyncTexture = 0;
		m_pSyncTexture = nullptr;
		m_hTextureMapLock = nullptr;
		m_pRTView = nullptr;		
		m_pSwapChain = nullptr;
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

	void Init(CTrackedHMD *pHmd);
	void Destroy();

	XMVECTOR m_ZAxis;

	ID3D11Device *m_pDevice;
	SharedTextureHandle_t m_SyncTexture;
	ID3D11Texture2D *m_pSyncTexture;

private:
	HWND m_hWnd;
	void CreateOutputWindow();
	bool m_IsRunning;

	D3D_FEATURE_LEVEL m_FeatureLevel;
	DWORD m_LastFrameTime;
	float m_FrameTime;

	ID3D11DeviceContext *m_pContext;

	CTrackedHMD *pHMD;

	float m_LeftRotate;
	float m_RightRotate;


	//ID3D11Texture2D *m_pRTTex;
	IDXGISwapChain* m_pSwapChain;
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

	LRESULT static CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
public:	
	HANDLE m_hTextureMapLock;
	std::vector<TextureSet*> m_TextureSets;
	std::map<SharedTextureHandle_t, TextureLink> m_TextureMap;
	TextureLink m_TlLeft;
	TextureLink m_TlRight;

	void RenderOutput();


};

#endif //DirectModeOutput_h

