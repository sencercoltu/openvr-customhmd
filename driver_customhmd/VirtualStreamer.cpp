#include "VirtualStreamer.h"
#include "TrackedHMD.h"


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "Dxgi.lib")

//#define ENABLE_TEXTURE_SS

#ifdef ENABLE_TEXTURE_SS
#include "C:\Program Files (x86)\Microsoft SDKs\DirectX\Include\D3DX11tex.h"
#ifdef _WIN64
#pragma comment(lib,"C:\\Program Files (x86)\\Microsoft SDKs\\DirectX\\Lib\\x64\\D3DX11.lib")
#else
#pragma comment(lib,"C:\\Program Files (x86)\\Microsoft SDKs\\DirectX\\Lib\\x86\\D3DX11.lib")
#endif
#endif //ENABLE_TEXTURE_SS

//
//HRESULT CMFTEventReceiver::QueryInterface(REFIID riid, void ** ppvObject)
//{
//	if (!ppvObject)
//		return E_FAIL;
//	if (riid == __uuidof(IUnknown))
//	{
//		*ppvObject = (IUnknown *)this;
//		return S_OK;
//	}
//	if (riid == __uuidof(IMFAsyncCallback))
//	{
//		*ppvObject = (IMFAsyncCallback *)this;
//		return S_OK;
//	}
//	*ppvObject = nullptr;
//	return E_NOINTERFACE;
//}
//
//ULONG CMFTEventReceiver::AddRef(void)
//{
//	return _refCount++;
//}
//
//ULONG CMFTEventReceiver::Release(void)
//{
//	if (_refCount > 0)
//		_refCount--;
//	if (!_refCount)
//		delete this;
//	return _refCount;
//}
//
//HRESULT STDMETHODCALLTYPE CMFTEventReceiver::GetParameters( /* [out] */ __RPC__out DWORD *pdwFlags, /* [out] */ __RPC__out DWORD *pdwQueue)
//{
//	return E_NOTIMPL;
//}
//HRESULT STDMETHODCALLTYPE CMFTEventReceiver::Invoke( /* [in] */ __RPC__in_opt IMFAsyncResult *pAsyncResult)
//{
//	CMFTEventReceiver *pReceiver = nullptr;
//	HRESULT hr = pAsyncResult->GetState((IUnknown **)&pReceiver);
//	IMFMediaEvent  *pEvent = nullptr;
//	IMFMediaEventGenerator *pGenerator = nullptr;
//	if (pReceiver)
//	{
//		pGenerator = pReceiver->m_pStreamer->m_pEventGenerator;
//		pGenerator->AddRef();
//	}
//
//	if (pGenerator)
//		pGenerator->EndGetEvent(pAsyncResult, &pEvent);
//
//
//	if (pEvent)
//	{
//		pReceiver->m_pStreamer->ProcessEvent(pEvent);
//		pEvent->Release();
//	}
//
//	if (pGenerator)
//	{
//		hr = pGenerator->BeginGetEvent(pReceiver, pReceiver);
//		pGenerator->Release();
//	}
//
//	if (pReceiver)	
//		pReceiver->Release();
//	
//
//	return S_OK;
//}
//

#define EXIT_IF_FAILED if (FAILED(hr)) { Destroy(); return; }

void VirtualStreamer::Init(CTrackedHMD *pHmd)
{
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	pHMD = pHmd;
	m_FrameTime = 1000.0f / pHmd->m_HMDData.Frequency;
	m_FrameCount = 0;

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
	};

	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0, //D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
		levels,
		ARRAYSIZE(levels),
		D3D11_SDK_VERSION,
		&m_pDevice,
		&m_FeatureLevel,
		&m_pContext);
	EXIT_IF_FAILED;

	//DirectMode disabled
	/*
	m_hBufferLock = CreateMutex(nullptr, FALSE, L"BufferLock");
	m_hTextureMapLock = CreateMutex(nullptr, FALSE, L"TextureMapLock");


	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = pHMD->m_HMDData.ScreenWidth;
	textureDesc.Height = pHMD->m_HMDData.ScreenHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // DXGI_FORMAT_R32G32B32A32_FLOAT; // seems amd h264 encoder supports this as input
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	//textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
	hr = m_pDevice->CreateTexture2D(&textureDesc, nullptr, &m_pRTTex);
	EXIT_IF_FAILED;

	ID3D11Texture2D *pCpuTexture;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = pHMD->m_HMDData.ScreenWidth;
	textureDesc.Height = pHMD->m_HMDData.ScreenHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // DXGI_FORMAT_R32G32B32A32_FLOAT; // seems amd h264 encoder supports this as input
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.BindFlags = 0;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	textureDesc.MiscFlags = 0;
	hr = m_pDevice->CreateTexture2D(&textureDesc, nullptr, &pCpuTexture);
	EXIT_IF_FAILED;


	hr = m_pDevice->CreateRenderTargetView(m_pRTTex, nullptr, &m_pRTView);
	EXIT_IF_FAILED;

	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;
	//cmdesc.FrontCounterClockwise = true;
	cmdesc.FrontCounterClockwise = false;
	hr = m_pDevice->CreateRasterizerState(&cmdesc, &m_pCWcullMode);
	EXIT_IF_FAILED;

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(CbEyePos);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	hr = m_pDevice->CreateBuffer(&cbbd, nullptr, &m_pConstantBuffer);
	EXIT_IF_FAILED;

	char shaderCode[] =
		"\
			cbuffer CbEyePos \
			{ \
				float4x4 SHIFT; \
			}; \
			Texture2D ObjTexture; \
			SamplerState ObjSamplerState; \
			struct VS_OUTPUT \
			{ \
				float4 Pos : SV_POSITION; \
				float2 TexCoord : TEXCOORD; \
			}; \
			VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD) \
			{ \
				VS_OUTPUT output; \
				output.Pos = mul(inPos, SHIFT); \
				output.TexCoord = inTexCoord; \
				return output; \
			} \
			float4 PS(VS_OUTPUT input) : SV_TARGET \
			{ \
				return ObjTexture.Sample(ObjSamplerState, input.TexCoord); \
			}";


	ID3DBlob *err = nullptr;
	hr = D3DCompile(shaderCode, strlen(shaderCode) + 1, nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_4_0", D3DCOMPILE_WARNINGS_ARE_ERRORS, 0, &m_pVS_Buffer, &err);
	EXIT_IF_FAILED;
	if (err)
		hr = err->Release();
	hr = m_pDevice->CreateVertexShader(m_pVS_Buffer->GetBufferPointer(), m_pVS_Buffer->GetBufferSize(), nullptr, &m_pVS);
	EXIT_IF_FAILED;

	hr = D3DCompile(shaderCode, strlen(shaderCode) + 1, nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_4_0", D3DCOMPILE_WARNINGS_ARE_ERRORS, 0, &m_pPS_Buffer, &err);
	EXIT_IF_FAILED;
	if (err)
		hr = err->Release();
	hr = m_pDevice->CreatePixelShader(m_pPS_Buffer->GetBufferPointer(), m_pPS_Buffer->GetBufferSize(), nullptr, &m_pPS);
	EXIT_IF_FAILED;

	Vertex v[] =
	{
		Vertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-1.0f,  1.0f, 0.0f, 0.0f, 0.0f),
		Vertex(0.0f,  1.0f, 0.0f, 1.0f, 0.0f),
		Vertex(0.0f, -1.0f, 0.0f, 1.0f, 1.0f)
	};

	DWORD indices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	hr = m_pDevice->CreateBuffer(&indexBufferDesc, &iinitData, &m_pSquareIndexBuffer);
	EXIT_IF_FAILED;

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_pSquareVertBuffer);
	EXIT_IF_FAILED;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = m_pDevice->CreateInputLayout(layout, numElements, m_pVS_Buffer->GetBufferPointer(), m_pVS_Buffer->GetBufferSize(), &m_pVertLayout);
	EXIT_IF_FAILED;

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)pHMD->m_HMDData.ScreenWidth;
	viewport.Height = (float)pHMD->m_HMDData.ScreenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Set the Viewport
	m_pContext->RSSetViewports(1, &viewport);


	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Create the Sample State
	hr = m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerState);
	EXIT_IF_FAILED;

	hr = m_pRTTex->QueryInterface(__uuidof(IDXGIKeyedMutex), (void **)&m_pTexSync);
	EXIT_IF_FAILED;

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pContext->IASetIndexBuffer(m_pSquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pContext->IASetVertexBuffers(0, 1, &m_pSquareVertBuffer, &stride, &offset);
	m_pContext->IASetInputLayout(m_pVertLayout);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->VSSetShader(m_pVS, 0, 0);
	m_pContext->PSSetShader(m_pPS, 0, 0);
	m_pContext->PSSetSamplers(0, 1, &m_pSamplerState);
	m_pContext->RSSetState(m_pCWcullMode);
	*/

	m_pServer = new CTCPServer(1974, TcpPacketReceive, this);

	m_hDisplayThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, RemoteDisplayThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hDisplayThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hDisplayThread);
	}
}

void VirtualStreamer::TextureFromHandle(SharedTextureHandle_t handle)
{
	ID3D11Texture2D *pTexture;

#ifdef ENABLE_TEXTURE_SS
	HRESULT hr = m_pDevice->OpenSharedResource((HANDLE)handle, __uuidof(ID3D11Texture2D), (void **)&pTexture);
	if (pTexture != nullptr)
	{
		IDXGIKeyedMutex *pMutex;
		if (SUCCEEDED(pTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void **)&pMutex)))
		{
			if (SUCCEEDED(pMutex->AcquireSync(0, 10)))
			{
				D3DX11SaveTextureToFile(m_pContext, pTexture, D3DX11_IFF_JPG, L"D:\\OUT\\IMG\\texture.jpg");
				pMutex->ReleaseSync(0);
			}
			pMutex->Release();
		}
		pTexture->Release();
	}
#endif //ENABLE_TEXTURE_SS

	//assume 3 buffers
	m_SequenceCounter++;
	TextureCache *pCache;

	int freeIndex = -1;
	for (int i = 0; i < 3; i++)
	{
		pCache = &m_TextureCache[i];
		if (pCache->m_hVirtualTexture == handle)
		{
			pCache->m_Sequence = m_SequenceCounter;
			m_TexIndex = i;
			m_FrameReady = true;
			return;
		}
		else if (!pCache->m_hVirtualTexture) 
		{
			freeIndex = i;
		}
	}
	if (freeIndex == -1) return;

	//empty slot @ i
	pCache = &m_TextureCache[freeIndex];
	HRESULT hr = m_pDevice->OpenSharedResource((HANDLE)handle, __uuidof(ID3D11Texture2D), (void **)&pTexture);
	if (pTexture)
	{
		AMF_RESULT res = m_pEncoderContext->CreateSurfaceFromDX11Native(pTexture, &pCache->m_pSurfaceTex, this);
		if (res == AMF_OK)
		{
			if (SUCCEEDED(pTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void **)&pCache->m_pTexSync)))
			{
				pCache->m_pVirtualTexture = pTexture;
				pCache->m_hVirtualTexture = handle;
				pCache->m_Sequence = m_SequenceCounter;
				m_TexIndex = freeIndex;
				m_FrameReady = true;
				return;
			}
		}
		m_FrameReady = false;
		pCache->m_pSurfaceTex = nullptr;
		pTexture->Release();
	}


}

void VirtualStreamer::Destroy()
{
	m_IsRunning = false;

	SAFE_THREADCLOSE(m_hDisplayThread);
	SAFE_RELEASE(m_pContext);
	SAFE_RELEASE(m_pDevice);

	if (m_pServer)
		delete m_pServer;
	m_pServer = nullptr;

	//DirectMode disabled
	/*
	SAFE_RELEASE(m_pSyncTexture);
	SAFE_CLOSE(m_hTextureMapLock);
	SAFE_CLOSE(m_hBufferLock);

	SAFE_RELEASE(m_pRTView);
	SAFE_RELEASE(m_pRTTex);
	SAFE_RELEASE(m_pTexSync);

	SAFE_RELEASE(m_pSquareIndexBuffer);
	SAFE_RELEASE(m_pSquareVertBuffer);
	SAFE_RELEASE(m_pVS);
	SAFE_RELEASE(m_pPS);
	SAFE_RELEASE(m_pVertLayout);
	SAFE_RELEASE(m_pVS_Buffer);
	SAFE_RELEASE(m_pPS_Buffer);

	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pCWcullMode);
	*/

	//MFT disabled
	//SAFE_RELEASE(m_pMediaBuffer);
	//SAFE_RELEASE(m_pInputSample);
	CoUninitialize();

	pHMD = nullptr;
}


unsigned int WINAPI VirtualStreamer::RemoteDisplayThread(void *p)
{
	auto dmd = static_cast<VirtualStreamer *>(p);
	if (dmd)
		dmd->RunRemoteDisplay();
	_endthreadex(0);
	return 0;
}

int inCount = 0;
int outCount = 0;
uint64_t encodeTime = 0;

void VirtualStreamer::RunRemoteDisplay()
{
	//HRESULT hr;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//UdpClientServer::CUdpClient *pUdpClient = new UdpClientServer::CUdpClient(m_HMDData.DirectStreamURL, 1974);
	InitEncoder();

	InfoPacket infoPacket = {};
	infoPacket.H = 'H';
	infoPacket.M = 'M';
	infoPacket.D1 = 'D';
	infoPacket.D2 = 'D';
	infoPacket.Width = pHMD->m_HMDData.ScreenWidth;
	infoPacket.Height = pHMD->m_HMDData.ScreenHeight;
	infoPacket.FrameRate = (int)pHMD->m_HMDData.Frequency;
	bool setHeaders = false;
	unsigned char *pFrameBuffer = (unsigned char *)malloc(8192 * 1024);
	int counter = 0;
	while (m_IsRunning)
	{		
		if (m_pServer->IsReady())
		{
			switch (m_DisplayState)
			{
			case 0:	//no connection				
				m_LastPacketReceive = 0;
				m_DisplayState = m_pServer->IsConnected() ? m_DisplayState + 1 : 0;
				Sleep(1); //failsafe
				continue;
			case 1: //new connected
				m_pServer->SendBuffer(VirtualPacketTypes::VrFrameInit, (const char*)&infoPacket, sizeof(infoPacket));
				Sleep(100);
				m_DisplayState = m_pServer->IsConnected() ? m_DisplayState + 1 : 0;
				Sleep(1); //failsafe
				continue;
			case 2:
				m_pEncoder->ReInit(pHMD->m_HMDData.ScreenWidth, pHMD->m_HMDData.ScreenHeight);
				setHeaders = true;
				m_DisplayState = m_pServer->IsConnected() ? m_DisplayState + 1 : 0;
				Sleep(1); //failsafe
				continue;
			case 3:
				m_DisplayState = m_pServer->IsConnected() ? m_DisplayState : 0;
				amf_pts now = amf_high_precision_clock();
				//if (((now - m_LastFrameTime) / 10000) < m_FrameTime)
				//{
				//	OutputDebugString(L"FrameSkip\n");
				//	continue;
				//}

				if (m_DisplayState && m_FrameReady && m_pServer->IsReady())
				{
//#ifdef _DEBUG
//					wchar_t dbg[1024];
//					wsprintf(dbg, L"In: %u Out: %u\n", inCount, outCount);
//					OutputDebugString(dbg);
//#endif //_DEBUG

					AMF_RESULT res;
					//amf_pts start_time;
					TextureCache *pCache = &m_TextureCache[m_TexIndex];
					if (pCache->m_Sequence != m_SequenceCounter)
					{
						OutputDebugString(L"Missed frame?\n");
					}
					if (SUCCEEDED(pCache->m_pTexSync->AcquireSync(0, 1)))
					{
						//if ((amf_high_precision_clock() - lastSPSPPS) / 10000000 >= 10)
						//{
						//	m_pEncoder->ReInit(pHMD->m_HMDData.ScreenWidth, pHMD->m_HMDData.ScreenHeight);
						//	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, true);
						//	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, true);
						//	lastSPSPPS = amf_high_precision_clock();
						//}
						m_LastFrameTime = now;
						//start_time = amf_high_precision_clock();
						if (setHeaders)
						{
							setHeaders = false;
							res = pCache->m_pSurfaceTex->SetProperty(AMF_VIDEO_ENCODER_INSERT_AUD, true);
							//res = pCache->m_pSurfaceTex->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, true);
							//res = pCache->m_pSurfaceTex->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, true);							
						}

						res = pCache->m_pSurfaceTex->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
						//res = pCache->m_pSurfaceTex->SetProperty(AMF_VIDEO_ENCODER_END_OF_STREAM, true);
						

						res = AMF_REPEAT;
						while (res == AMF_REPEAT)
						{
							res = m_pEncoder->SubmitInput(pCache->m_pSurfaceTex);
							inCount++;
						}
						//m_EndcodeElapsed += amf_high_precision_clock() - start_time;
						m_FrameReady = false;
						pCache->m_pTexSync->ReleaseSync(0);
					}
					else
					{
						OutputDebugString(L"NoLock \n");
						Sleep(1); //failsafe
						continue;
					}

					amf::AMFDataPtr data;
					//start_time = amf_high_precision_clock();
					res = m_pEncoder->QueryOutput(&data);
					//m_EndcodeElapsed += amf_high_precision_clock() - start_time;					
					
					if (res == AMF_OK)
					{
						amf::AMFBufferPtr buffer(data);
						unsigned char *pData = (unsigned char *)buffer->GetNative();
						int len = (int)buffer->GetSize();
						if (len > 0)
						{
							outCount++;
							memcpy(pFrameBuffer, pData, len);
							m_pServer->SendBuffer(VirtualPacketTypes::VrFrame, (const char *)pFrameBuffer, len);
							/*FILE *fp = nullptr;
							fopen_s(&fp, "D:\\OUT\\xx.h264", "a");
							if (fp)
							{
								fwrite(pFrameBuffer, 1, len, fp);
								fclose(fp);
							}*/
							//OutputDebugString(L"Start send\n");
						}
					}					
					//else
					//{
					//	counter++;
					//}

					//MFT disabled
					//add raw frame to encoder and send encoded 
					//auto pEvent = GetEvent();
					//if (pEvent)
					//{
					//	ProcessEvent(pEvent, pServer);
					//	pEvent->Release();
					//}
				}
				else
				{
					//OutputDebugString(m_FrameReady ? L"Server Not ready!\n" : L"Frame Not ready!\n");
					Sleep(1); //failsafe
				}
			}
		}
	}

	DestroyEncoder();

	if (pFrameBuffer)
		free(pFrameBuffer);
	pFrameBuffer = nullptr;

	//if (pUdpClient)
	//	delete pUdpClient;
	//pUdpClient = nullptr;

	WSACleanup();
}

/*
void VirtualStreamer::ProcessEvent(IMFMediaEvent *pEvent, CTCPServer *pServer)
{
	HRESULT hr;
	MediaEventType type;
	pEvent->GetType(&type);
	//pEvent->GetStatus(&hr);
	//PROPVARIANT val;
	//PropVariantInit(&val);
	//pEvent->GetValue(&val);
	//PropVariantClear(&val);
	switch (type)
	{
		case METransformNeedInput:

			m_pInputSample->SetSampleTime(GetTickCount64());
			//DWORD cnt = 0;
			//hr = m_pInputSample->GetBufferCount(&cnt);
			//IMFMediaBuffer *pBuffer = nullptr;
			//hr = m_pInputSample->GetBufferByIndex(0, &pBuffer);
			if (SUCCEEDED(m_pTexSync->AcquireSync(0, 10)))
			{
				m_pContext->CopyResource(m_pTexBuffer, m_pRTTex);
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				hr = m_pContext->Map(m_pTexBuffer, 0, D3D11_MAP::D3D11_MAP_READ, 0, &mappedResource);
				IMFMediaBuffer *pMediaBuffer;
				m_pInputSample->GetBufferByIndex(0, &pMediaBuffer);
				BYTE *pData;
				DWORD dwLen, dwMax;
				hr = pMediaBuffer->Lock(&pData, &dwMax, &dwLen);
				memcpy(pData, mappedResource.pData, mappedResource.RowPitch * pHMD->m_HMDData.ScreenHeight);
				hr = pMediaBuffer->Unlock();
				m_pContext->Unmap(m_pTexBuffer, 0);
				hr = m_pTransform->ProcessInput(0, m_pInputSample, 0);
				//copy tex buffer to sample buffer :(
				//pBuffer->QueryInterface
				//OutputDebugString(L"IN\n");
				//IMFMediaBuffer *pSample;
				//hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), m_pRTTex, 0, FALSE, &pSample);
				//if (pSample)
				//{
				//	IMFMediaBuffer *pBuffer;
				//	hr = m_pInputSample->GetBufferByIndex(0, &pBuffer);
				//	BYTE *pSrc, *pDst;
				//	DWORD srcLen, dstLen, srcMax, dstMax;

				//	hr = pSample->Lock(&pSrc, &srcMax, &srcLen);
				//	hr = pBuffer->Lock(&pDst, &dstMax, &dstLen);
				//	memcpy(pDst, pSrc, min(srcLen, dstMax));
				//	hr = pBuffer->SetCurrentLength(srcLen);
				//	hr = pBuffer->Unlock();
				//	hr = pSample->Unlock();
				//
				//	hr = m_pTransform->ProcessInput(0, m_pInputSample, 0);
				//	hr = pSample->Release();
				//	hr = pBuffer->Release();
				//}
				m_pTexSync->ReleaseSync(0);
			}
			//pBuffer->Release();
			break;
		case METransformHaveOutput:
		{
			DWORD status;
			MFT_OUTPUT_DATA_BUFFER outBuf = { 0 };
			outBuf.pSample = m_pOutputSample;
			//OutputDebugString(L"OUT\n");
			hr = m_pTransform->ProcessOutput(0, 1, &outBuf, &status);
			if (SUCCEEDED(hr))
			{
				m_pOutputSample = outBuf.pSample; //release at exit
				if (outBuf.pEvents)
					outBuf.pEvents->Release();
			}
			IMFMediaBuffer *pBuffer;
			hr = m_pOutputSample->GetBufferByIndex(0, &pBuffer);
			if (pBuffer)
			{
				DWORD currLen, maxLen;
				unsigned char *pData = nullptr;;
				if (SUCCEEDED(pBuffer->Lock(&pData, &maxLen, &currLen)))
				{
					if (pData && currLen)
					{
						pServer->SendBuffer((const char *)&currLen, sizeof(int));
						pServer->SendBuffer((const char *)pData, currLen);
						if (m_FileDump) fwrite(pData, 1, currLen, m_FileDump);
					}
					pBuffer->Unlock();
				}
				pBuffer->Release();
			}


			break;
		}
	}
}
*/


void VirtualStreamer::EnableCamera(bool enable)
{
	if (enable != m_LastCameraStatus)
	{
		m_LastCameraStatus = enable;
		//do some shit
		if (m_pServer)
		{
			InitDecoder();
			m_pServer->SetCameraStatus(&m_LastCameraStatus);
		}
		else
			DestroyDecoder();
	}
}

//int pCounter = 0;

void VirtualStreamer::TcpPacketReceive(void *dst, VirtualPacketTypes type, const char *pData, int len)
{
	VirtualStreamer *pDM = (VirtualStreamer *)dst;
	if (pData == nullptr)
	{
		pDM->m_DisplayState = 0;
		return;
	}

	switch (type)
	{
	case VirtualPacketTypes::Rotation:
	{

		USBPacket *pPacket = (USBPacket *)pData;
		SetPacketCrc(pPacket);
		pDM->ProcessRemotePacket(pPacket);
		pDM->m_LastPacketReceive = GetTickCount();
		//pCounter++;
		//if (!(pCounter % 100))
		//{
		//	OutputDebugString(L"Got 100 rots...\n");
		//	pCounter = 0;
		//}
		break;
	}
	case VirtualPacketTypes::CameraFrameInit:
	{
		//reset decoder
		auto pInfoPacket = (InfoPacket *)pData;

		break;
	}
	case VirtualPacketTypes::CameraFrame:
	{
		auto pCameraOptions = &pDM->pHMD->m_Camera.Options;
		//decode frame to pCameraOptions->pCaptureBuffer



		if (pCameraOptions && pCameraOptions->pfCallback)
			pCameraOptions->pfCallback(nullptr, pCameraOptions->pUserData);
		break;
	}
	}
}

void VirtualStreamer::ProcessRemotePacket(USBPacket *pPacket)
{
	pHMD->m_pServer->ProcessUSBPacket(pPacket);
}

#define EXIT_AND_DESTROY if (res != AMF_OK) { DestroyEncoder(); return false;}

#ifdef _DEBUG
//#define DEBUG_AMF
#endif //_DEBUG

bool VirtualStreamer::InitEncoder()
{
	AMF_RESULT res = AMF_OK;
	res = g_AMFFactory.Init();
	if (res != AMF_OK)
		return false;

	amf_increase_timer_precision();

	res = g_AMFFactory.GetFactory()->CreateContext(&m_pEncoderContext);
	EXIT_AND_DESTROY;

	res = m_pEncoderContext->InitDX11(m_pDevice, amf::AMF_DX11_1); // can be DX11 device
	EXIT_AND_DESTROY;

	res = g_AMFFactory.GetFactory()->CreateComponent(m_pEncoderContext, AMFVideoEncoderVCE_AVC, &m_pEncoder);
	EXIT_AND_DESTROY;
	
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, 0);
	
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(pHMD->m_HMDData.ScreenWidth, pHMD->m_HMDData.ScreenHeight));
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate((amf_uint32)pHMD->m_HMDData.Frequency, 1));
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_BASELINE);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_SCANTYPE, AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, 0);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, false);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, 4);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, 1024 * 100);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, 5);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, 20);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, 10);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, 50);
	//res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, 16);


#ifdef DEBUG_AMF
	auto m_AMFDebug = g_AMFFactory.GetDebug();
	m_AMFDebug->EnablePerformanceMonitor(true);
	//m_AMFDebug->AssertsEnable(true);
	auto m_AMFTrace = g_AMFFactory.GetTrace();
	m_AMFTrace->SetGlobalLevel(AMF_TRACE_TEST);
	m_AMFTrace->TraceEnableAsync(true);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TEST);
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, true);
#endif //DEBUG_AMF




	res = m_pEncoder->Init(amf::AMF_SURFACE_RGBA, pHMD->m_HMDData.ScreenWidth, pHMD->m_HMDData.ScreenHeight);
	EXIT_AND_DESTROY;

	auto bitRate = (int)((pHMD->m_HMDData.ScreenWidth * pHMD->m_HMDData.ScreenHeight * pHMD->m_HMDData.ScreenWidth * pHMD->m_HMDData.Frequency) );

//#define ENCODE_BITRATE (1024*1024*30)
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, false);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, 1);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, 0);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitRate);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, bitRate);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_BASELINE);
	////res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 50);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, true);

	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, 10);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, 10);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, 22);



	

	return true;
}

void VirtualStreamer::DestroyEncoder()
{
	//if (m_pSurfaceIn)
	//	m_pSurfaceIn->Release();
	m_TexIndex = -1;
	for (int i = 0; i < 3; i++)
	{
		m_TextureCache[i].m_pSurfaceTex = nullptr;
		m_TextureCache[i].m_hVirtualTexture = 0;
		SAFE_RELEASE(m_TextureCache[i].m_pVirtualTexture);
		SAFE_RELEASE(m_TextureCache[i].m_pTexSync);
	}

	if (m_pEncoder)
		m_pEncoder->Terminate();
	m_pEncoder = nullptr;

	if (m_pEncoderContext)
		m_pEncoderContext->Terminate();
	m_pEncoderContext = nullptr;
	//#ifdef _DEBUG
	//	amf::AMFTraceEnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, false);
	//#endif //_DEBUG
	amf_restore_timer_precision();
	g_AMFFactory.Terminate();
}



bool VirtualStreamer::InitDecoder()
{
	AMF_RESULT res = AMF_OK;



	return true;
}

void VirtualStreamer::DestroyDecoder()
{

}



/*
#define EXIT_IF_FAILED_MFT if (FAILED(hr)) { DestroyMFT(); return false; }

bool VirtualStreamer::InitMFT()
{
	m_pEventGenerator = nullptr;
	m_pTransform = nullptr;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	hr = MFStartup(MF_VERSION);

	hr = MFTRegisterLocalByCLSID(__uuidof(CColorConvertDMO), MFT_CATEGORY_VIDEO_PROCESSOR, L"", MFT_ENUM_FLAG_HARDWARE, 0, NULL, 0, NULL);

	MFT_REGISTER_TYPE_INFO inp = { MFMediaType_Video, MFVideoFormat_ARGB32 };
	MFT_REGISTER_TYPE_INFO outp = { MFMediaType_Video, MFVideoFormat_H264 };
	UINT32 nCount = 0;
	IMFActivate **ppActivate = nullptr;

	//find hardware encoders
	hr = MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER, MFT_ENUM_FLAG_HARDWARE, &inp, &outp, &ppActivate, &nCount);
	if (nCount == 0)
	{
		nCount = 0; //fallback to software
		hr = MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER, MFT_ENUM_FLAG_ASYNCMFT, &inp, &outp, &ppActivate, &nCount);
		if (nCount == 0)
		{
			Destroy();
			return false;
		}
	}


	//select first
	hr = ppActivate[0]->ActivateObject(IID_PPV_ARGS(&m_pTransform));

	//and unref all
	for (UINT32 i = 0; i < nCount; i++)
	{
		ppActivate[i]->Release();
	}
	CoTaskMemFree(ppActivate);

	EXIT_IF_FAILED_MFT;


	//unlock async mft
	IMFAttributes *pAttributes = nullptr;
	hr = m_pTransform->GetAttributes(&pAttributes);
	EXIT_IF_FAILED_MFT;

	BOOL bAware = MFGetAttributeUINT32(pAttributes, MF_SA_D3D11_AWARE, FALSE);

	BOOL bAsync = MFGetAttributeUINT32(pAttributes, MF_TRANSFORM_ASYNC, FALSE);
	if (bAsync) pAttributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);


	hr = pAttributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);


	hr = pAttributes->SetUINT32(CODECAPI_AVEncMPVDefaultBPictureCount, 0);
	hr = pAttributes->SetUINT32(CODECAPI_AVEncCommonRateControlMode, eAVEncCommonRateControlMode_CBR);
	hr = pAttributes->SetUINT32(CODECAPI_AVEncCommonQuality, 70);
	hr = pAttributes->SetUINT32(CODECAPI_AVEncMPVGOPSize, 2);
	hr = pAttributes->SetUINT32(CODECAPI_AVLowLatencyMode, TRUE);

	hr = pAttributes->SetUINT32(MFT_PREFERRED_ENCODER_PROFILE, eAVEncH264VProfile_Base);
	hr = pAttributes->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
	hr = pAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);


	pAttributes->Release();
	pAttributes = nullptr;

	if (!bAware)
	{
		Destroy();
		return false;
	}


	int Width = pHMD->m_HMDData.ScreenWidth;
	int Height = pHMD->m_HMDData.ScreenHeight;
	int FPS = (int)pHMD->m_HMDData.Frequency;

	DWORD inMin, inMax, outMin, outMax;
	hr = m_pTransform->GetStreamLimits(&inMin, &inMax, &outMin, &outMax);

	UINT m_ManagerToken = 0;
	IMFDXGIDeviceManager *m_pManager = nullptr;
	hr = MFCreateDXGIDeviceManager(&m_ManagerToken, &m_pManager);
	hr = m_pManager->ResetDevice(m_pDevice, m_ManagerToken);
	EXIT_IF_FAILED_MFT;



	//not implemented
	//DWORD inIds[16] = { 0xffffffff };
	//DWORD outIds[16] = { 0xffffffff };
	//hr = m_pTransform->GetStreamIDs(16, inIds, 16, outIds);

	IMFMediaType *pMFTInputMediaType = nullptr, *pMFTOutputMediaType = nullptr;

	hr = MFCreateMediaType(&pMFTOutputMediaType);
	hr = pMFTOutputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = pMFTOutputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
	hr = pMFTOutputMediaType->SetUINT32(MF_MT_AVG_BITRATE, 900000);
	hr = MFSetAttributeSize(pMFTOutputMediaType, MF_MT_FRAME_SIZE, Width, Height);
	hr = MFSetAttributeRatio(pMFTOutputMediaType, MF_MT_FRAME_RATE, FPS, 1);
	hr = MFSetAttributeRatio(pMFTOutputMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	hr = pMFTOutputMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	hr = pMFTOutputMediaType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_Base);
	hr = pMFTOutputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
	hr = pMFTOutputMediaType->SetUINT32(MF_LOW_LATENCY, TRUE);
	hr = m_pTransform->SetOutputType(0, pMFTOutputMediaType, 0);
	EXIT_IF_FAILED_MFT;

	//hr = m_pTransform->GetInputAvailableType(1, 1, &pMFTInputMediaType);

	hr = MFCreateMediaType(&pMFTInputMediaType);
	hr = pMFTInputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = pMFTInputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
	hr = MFSetAttributeSize(pMFTInputMediaType, MF_MT_FRAME_SIZE, Width, Height);
	hr = MFSetAttributeRatio(pMFTInputMediaType, MF_MT_FRAME_RATE, FPS, 1);
	hr = MFSetAttributeRatio(pMFTInputMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	hr = pMFTInputMediaType->SetUINT32(MF_SA_D3D11_USAGE, D3D11_USAGE_DEFAULT);
	hr = pMFTInputMediaType->SetUINT32(MF_SA_D3D11_BINDFLAGS, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VIDEO_ENCODER);
	hr = pMFTInputMediaType->SetUINT32(MF_SA_D3D11_SHARED, TRUE);
	hr = m_pTransform->SetInputType(0, pMFTInputMediaType, 0);
	//pMFTInputMediaType->Release();
	EXIT_IF_FAILED_MFT;


	//IMFVideoSampleAllocatorEx *pAllocator;
	//hr = MFCreateVideoSampleAllocatorEx(__uuidof(IMFVideoSampleAllocatorEx), (void **)&pAllocator);
	//EXIT_IF_FAILED_MFT;
	//
	//hr = MFCreateAttributes(&pAttributes, 2);
	//EXIT_IF_FAILED_MFT;
	//hr = pAllocator->SetDirectXManager(m_pManager);
	//hr = pAllocator->InitializeSampleAllocatorEx(1, 1, pAttributes, pMFTInputMediaType);
	//
	//pAttributes->Release();
	//EXIT_IF_FAILED_MFT;
	//
	//hr = pAllocator->AllocateSample(&m_pInputSample);
	//pAllocator->Release();
	//EXIT_IF_FAILED_MFT;

	//hr = MFCreateVideoSampleFromSurface(m_pRTTex, &m_pInputSample);

	//UINT32 cnt;
	//m_pInputSample->GetCount(&cnt);
	//for (auto i = 0; i < cnt; i++)
	//{
	//	GUID key;
	//	PROPVARIANT v;
	//	PropVariantInit(&v);
	//	m_pInputSample->GetItemByIndex(i, &key, &v);
	//	PropVariantClear(&v);
	//}
	//m_pInputSample->Release();

	// Create the DirectX surface buffer


	IMFMediaBuffer *pBuffer;
	//hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), m_pRTTex, 0, false, &pBuffer);
	//IMF2DBuffer *p2DBuffer;
	//hr = pBuffer->QueryInterface(__uuidof(IMF2DBuffer), (void**)&p2DBuffer);
	//DWORD Length;
	//hr = p2DBuffer->GetContiguousLength(&Length);
	//p2DBuffer->Release();

	//// Set the data length of the buffer
	//hr = pBuffer->SetCurrentLength(Length);

	//// Create a media sample and add the buffer to the sample
	hr = MFCreateSample(&m_pInputSample);
	////m_pInputSample->CopyAllItems(pMFTInputMediaType);

	hr = MFCreate2DMediaBuffer(Width, Height, D3DFMT_A8R8G8B8, FALSE, &pBuffer);
	hr = m_pInputSample->AddBuffer(pBuffer);
	// Set the time stamp and the duration
	pBuffer->Release();


	hr = pMFTInputMediaType->Release();
	hr = pMFTOutputMediaType->Release();

	//IMFSample *pSample;
	//hr = MFCreateVideoSampleFromSurface(m_pRTTex, &pSample);
	//EXIT_IF_FAILED_MFT;

	//m_pInputSample = new CIdidotAMDSampleFix(pSample);

	hr = m_pInputSample->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = m_pInputSample->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
	hr = m_pInputSample->SetUINT32(MF_MT_DEFAULT_STRIDE, 4 * Width);
	hr = MFSetAttributeSize(m_pInputSample, MF_MT_FRAME_SIZE, Width, Height);



	m_pInputSample->SetSampleDuration((LONGLONG)m_FrameTime);
	m_pInputSample->SetSampleTime(GetTickCount64());



	MFT_OUTPUT_STREAM_INFO info;
	hr = m_pTransform->GetOutputStreamInfo(0, &info);

	DWORD mftStatus = 0;

	hr = m_pTransform->GetInputStatus(0, &mftStatus);

	if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus)
	{
		Destroy();
		return false;
	}


	hr = m_pTransform->QueryInterface(IID_IMFMediaEventGenerator, (void**)&m_pEventGenerator);
	EXIT_IF_FAILED_MFT;

	hr = m_pTransform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, (ULONG_PTR)m_pManager);
	EXIT_IF_FAILED_MFT;




	hr = m_pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0); EXIT_IF_FAILED_MFT;
	hr = m_pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0); EXIT_IF_FAILED_MFT;
	hr = m_pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0); EXIT_IF_FAILED_MFT;

	//encoder ready

	return true;
}

void VirtualStreamer::DestroyMFT()
{
	//if (m_pEventGenerator)
	//	m_pEventGenerator->EndGetEvent(this, nullptr);

	if (m_pTransform)
	{
		m_pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
		m_pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
	}

	if (m_pManager && m_pDevice)
		m_pManager->ResetDevice(m_pDevice, m_ManagerToken);
	m_ManagerToken = 0;
	SAFE_RELEASE(m_pManager);
	SAFE_RELEASE(m_pTransform);
	SAFE_RELEASE(m_pEventGenerator);
	SAFE_RELEASE(m_pInputSample);
	SAFE_RELEASE(m_pOutputSample);
	fopen_s(&m_FileDump, "D:\\OUT\\xx.h264", "wb");
	if (m_FileDump)
		fclose(m_FileDump);
	m_FileDump = nullptr;
}

IMFMediaEvent *VirtualStreamer::GetEvent()
{
	if (!m_pEventGenerator) return nullptr;
	IMFMediaEvent *pEvent = nullptr;
	if (SUCCEEDED(m_pEventGenerator->GetEvent(MF_EVENT_FLAG_NO_WAIT, &pEvent)))
		return pEvent;
	return nullptr;
}
*/