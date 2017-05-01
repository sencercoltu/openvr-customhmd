#include "DirectModeStreamer.h"
#include "TrackedHMD.h"

//#define ENABLE_COMBINED_SS


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "Dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "Evr.lib")


#ifdef ENABLE_COMBINED_SS
#include "C:\Program Files (x86)\Microsoft SDKs\DirectX\Include\D3DX11tex.h"
#ifdef _WIN64
#pragma comment(lib,"C:\\Program Files (x86)\\Microsoft SDKs\\DirectX\\Lib\\x64\\D3DX11.lib")
#else
#pragma comment(lib,"C:\\Program Files (x86)\\Microsoft SDKs\\DirectX\\Lib\\x86\\D3DX11.lib")
#endif
#endif //ENABLE_COMBINED_SS
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

void DirectModeStreamer::Init(CTrackedHMD *pHmd)
{

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	pHMD = pHmd;
	m_hBufferLock = CreateMutex(nullptr, FALSE, L"BufferLock");
	m_hTextureMapLock = CreateMutex(nullptr, FALSE, L"TextureMapLock");

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_11_1
	};

	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
		levels,
		ARRAYSIZE(levels),
		D3D11_SDK_VERSION,
		&m_pDevice,
		&m_FeatureLevel,
		&m_pContext);
	EXIT_IF_FAILED;	
	
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

	hr = pCpuTexture->QueryInterface(__uuidof(ID3D11Resource), (void **)&m_pTexBuffer);
	pCpuTexture->Release();
	EXIT_IF_FAILED;

	//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	//ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	//renderTargetViewDesc.Format = textureDesc.Format;
	//renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//renderTargetViewDesc.Texture2D.MipSlice = 0;	

	hr = m_pDevice->CreateRenderTargetView(m_pRTTex, /*&renderTargetViewDesc*/ nullptr, &m_pRTView);
	EXIT_IF_FAILED;

	m_pRTView->GetResource(&m_pRTRes);


	//D3D11_BUFFER_DESC bufDesc = {};
	//bufDesc.Usage = D3D11_USAGE_STAGING;
	//bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	//bufDesc.StructureByteStride = 1;
	//bufDesc.ByteWidth = pHMD->m_HMDData.ScreenWidth * pHMD->m_HMDData.ScreenHeight * sizeof(float);
	//hr = m_pDevice->CreateBuffer(&bufDesc, nullptr, &m_pTexBuffer);
	//EXIT_IF_FAILED;
	//
/*
	ID2D1Factory* m_pDirect2dFactory;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pDirect2dFactory);
	EXIT_IF_FAILED;

	IDXGISurface2 *pSurface;
	hr = m_pRTTex->QueryInterface(__uuidof(IDXGISurface2), (void**)&pSurface);
	EXIT_IF_FAILED;

	D2D1_RENDER_TARGET_PROPERTIES props = {};
	props.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
	props.pixelFormat.format = textureDesc.Format;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;


	ID2D1RenderTarget *pTarget;
	hr = m_pDirect2dFactory->CreateDxgiSurfaceRenderTarget(pSurface, props, &pTarget);
*/

/*
	hr = m_pInputSample->RemoveAllBuffers();
		
	IMFMediaBuffer *pBuffer = nullptr;
	hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), m_pRTTex, 0, FALSE, &pBuffer);
	EXIT_IF_FAILED;	

	hr = m_pInputSample->AddBuffer(pBuffer);
	pBuffer->Release();
	EXIT_IF_FAILED;
*/



	//IDXGIDevice *pDxgiDevice = nullptr;
	//hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDxgiDevice);


	//hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), m_pRTTex, 0, FALSE, &m_pMediaBuffer);
	//EXIT_IF_FAILED;



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

	m_hDisplayThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, RemoteDisplayThread, this, CREATE_SUSPENDED, nullptr));
	if (m_hDisplayThread)
	{
		m_IsRunning = true;
		ResumeThread(m_hDisplayThread);
	}

}

void DirectModeStreamer::Destroy()
{
	m_IsRunning = false;

	SAFE_THREADCLOSE(m_hDisplayThread);
	SAFE_RELEASE(m_pSyncTexture);
	SAFE_RELEASE(m_pContext);
	SAFE_RELEASE(m_pDevice);
	SAFE_CLOSE(m_hTextureMapLock);
	SAFE_CLOSE(m_hBufferLock);

	SAFE_RELEASE(m_pRTRes);		
	SAFE_RELEASE(m_pRTView);
	SAFE_RELEASE(m_pRTTex);
	SAFE_RELEASE(m_pTexSync);

	//SAFE_RELEASE(m_pMediaBuffer);

	SAFE_RELEASE(m_pTexBuffer);
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

	SAFE_RELEASE(m_pInputSample);

	CoUninitialize();

	pHMD = nullptr;
}

void DirectModeStreamer::CombineEyes()
{
	if (!m_FrameTime || !m_pTexSync) return;
	ID3D11ShaderResourceView *pEmptyTexture[1] = { nullptr };
	DWORD now = GetTickCount();
	if (now - m_LastFrameTime < m_FrameTime)
		return;
	m_LastFrameTime = now;



	if (SUCCEEDED(m_pTexSync->AcquireSync(0, 10)))
	{

		float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
		m_pContext->ClearRenderTargetView(m_pRTView, bgColor);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		//if (m_TlLeft.pData && m_TlLeft.pData->pGPUTexture) D3DX11SaveTextureToFile(m_pContext, m_TlLeft.pData->pGPUTexture, D3DX11_IFF_JPG, L"D:\\OUT\\IMG\\file-left.jpg");
		//if (m_TlRight.pData && m_TlRight.pData->pGPUTexture) D3DX11SaveTextureToFile(m_pContext, m_TlRight.pData->pGPUTexture, D3DX11_IFF_JPG, L"D:\\OUT\\IMG\\file-right.jpg");


		m_pContext->OMSetRenderTargets(1, &m_pRTView, nullptr);

		m_pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pContext->IASetIndexBuffer(m_pSquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_pContext->IASetVertexBuffers(0, 1, &m_pSquareVertBuffer, &stride, &offset);
		m_pContext->IASetInputLayout(m_pVertLayout);
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pContext->VSSetShader(m_pVS, 0, 0);
		m_pContext->PSSetShader(m_pPS, 0, 0);
		m_pContext->PSSetSamplers(0, 1, &m_pSamplerState);
		m_pContext->RSSetState(m_pCWcullMode);

		//draw left texture to combined texture
		if (m_TlLeft.pData)
		{
			Ep.SHIFT = XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, 0.0f));
			m_pContext->PSSetShaderResources(0, 1, &m_TlLeft.pData->pShaderResourceView);
			m_pContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &Ep, 0, 0);
			m_pContext->DrawIndexed(6, 0, 0);
			m_pContext->PSSetShaderResources(0, 1, pEmptyTexture);
			m_TlLeft.pData = nullptr;
		}

		//draw right texture to combined texture
		if (m_TlRight.pData)
		{
			Ep.SHIFT = XMMatrixTranspose(XMMatrixTranslation(1.0f, 0.0f, 0.0f));
			m_pContext->PSSetShaderResources(0, 1, &m_TlRight.pData->pShaderResourceView);
			m_pContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &Ep, 0, 0);
			m_pContext->DrawIndexed(6, 0, 0);
			m_pContext->PSSetShaderResources(0, 1, pEmptyTexture);
			m_TlRight.pData = nullptr;
		}


#ifdef ENABLE_COMBINED_SS
		ID3D11Resource *pTexRes = nullptr;
		m_pRTTex->QueryInterface(__uuidof(ID3D11Resource), (void**)&pTexRes);
		if (!pTexRes)
			return;

		D3DX11SaveTextureToFile(m_pContext, pTexRes, D3DX11_IFF_JPG, L"D:\\OUT\\IMG\\file.jpg");

		pTexRes->Release();
#endif //ENABLE_COMBINED_SS
		m_pTexSync->ReleaseSync(0);
	}
}


unsigned int WINAPI DirectModeStreamer::RemoteDisplayThread(void *p)
{
	auto dmd = static_cast<DirectModeStreamer *>(p);
	if (dmd)
		dmd->RunRemoteDisplay();
	_endthreadex(0);
	return 0;
}

void DirectModeStreamer::RunRemoteDisplay()
{
	//HRESULT hr;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	CTCPServer *pServer = new CTCPServer(1974, TcpPacketReceive, this);
	//UdpClientServer::CUdpClient *pUdpClient = new UdpClientServer::CUdpClient(m_HMDData.DirectStreamURL, 1974);

	//m_DirectScreen.Init(pHMD->m_HMDData.ScreenWidth, pHMD->m_HMDData.ScreenHeight, pHMD->m_HMDData.Frequency, pHMD->m_HMDData.DirectStreamURL);
	InitMFT();


	InfoPacket infoPacket = {};
	infoPacket.H = 'H';
	infoPacket.M = 'M';
	infoPacket.D1 = 'D';
	infoPacket.D2 = 'D';
	infoPacket.Width = pHMD->m_HMDData.ScreenWidth;
	infoPacket.Height = pHMD->m_HMDData.ScreenHeight;

	//CMFTEventReceiver *pEventReceiver = nullptr;
	//if (m_pEventGenerator)
	//{
	//	pEventReceiver = new CMFTEventReceiver(this);
	//	hr = m_pEventGenerator->BeginGetEvent(pEventReceiver, pEventReceiver);
	//}



	FILE *fp = nullptr;
	//fopen_s(&fp, "D:\\test.h264", "wb");	
	while (m_IsRunning)
	{
		Sleep(1);
		if (/*m_Streamer.pPixelBuffer[0] && */pServer->IsReady())
		{
			switch (m_DisplayState)
			{
			case 0:	//no connection				
				m_LastPacketReceive = 0;
				m_DisplayState = pServer->IsConnected() ? m_DisplayState + 1 : 0;
				continue;
			case 1: //new connected
				pServer->SendBuffer((const char*)&infoPacket, sizeof(infoPacket));
				Sleep(100);
				m_DisplayState = pServer->IsConnected() ? m_DisplayState + 1 : 0;
				continue;
			case 2:
				//pServer->SendBuffer((const char*)m_DirectScreen.pCodecContext->extradata, m_DirectScreen.pCodecContext->extradata_size);
				//if (fp) fwrite((const char*)m_DirectScreen.pCodecContext->extradata, 1, m_DirectScreen.pCodecContext->extradata_size, fp);
				m_DisplayState = pServer->IsConnected() ? m_DisplayState + 1 : 0;
				continue;
			case 3:
				m_DisplayState = pServer->IsConnected() ? m_DisplayState : 0;

				if (pServer->IsReady())
				{				
					//add raw frame to encoder and send encoded 
					auto pEvent = GetEvent();
					if (pEvent)
					{
						ProcessEvent(pEvent, pServer);
						pEvent->Release();
					}
				}



				//if (WAIT_OBJECT_0 == WaitForSingleObject(m_hTextureMapLock, 10))
				//{
				//	pitch += UpdateBuffer(&m_Streamer.Left);
				//	pitch += UpdateBuffer(&m_Streamer.Right);
				//	ReleaseMutex(m_hTextureMapLock);
				//}

				//if (!pitch ||
				//	m_Streamer.Left.Desc.Width != m_Streamer.Right.Desc.Width || //left and right res should be same
				//	m_Streamer.Left.Desc.Height != m_Streamer.Right.Desc.Height ||
				//	m_Streamer.Left.Desc.Format != m_Streamer.Right.Desc.Format)
				//{
				//	Sleep(1);
				//	continue;
				//}

				//if (!m_DirectScreen.ProcessFrame(pitch))
				//{
				//	//while (auto pPacket = m_DirectScreen.GetPacket())
				//	//{
				//	//	auto remSize = pPacket->size;
				//	//	auto *pData = (const char *)pPacket->data;
				//	//	//pUdpClient->send((const char*)pData, remSize);
				//	//	pServer->SendBuffer(pData, remSize);
				//	//	if (fp) fwrite(pData, 1, remSize, fp);
				//	//}
				//}
			}
			Sleep(1);
		}
		else
			Sleep(100);
	}

	//m_DirectScreen.Destroy();

	if (fp) fclose(fp);
	fp = nullptr;

	delete pServer;
	pServer = nullptr;

	//if (pEventReceiver)
	//{
	//	pEventReceiver->Release();
	//	pEventReceiver = nullptr;
	//}

	//delete pUdpClient;
	//pUdpClient = nullptr;
	WSACleanup();
}

void DirectModeStreamer::ProcessEvent(IMFMediaEvent *pEvent, CTCPServer *pServer)
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



void DirectModeStreamer::TcpPacketReceive(void *dst, const char *pData, int len)
{
	DirectModeStreamer *pDM = (DirectModeStreamer *)dst;
	if (pData == nullptr)
	{
		pDM->m_DisplayState = 0;
		return;
	}
	if (len != 32) return;
	//read only last 32 bytes, discard others for now
	USBPacket *pPacket = (USBPacket *)pData;
	SetPacketCrc(pPacket);
	pDM->ProcessRemotePacket(pPacket);
	pDM->m_LastPacketReceive = GetTickCount();
}

int DirectModeStreamer::UpdateBuffer(DirectEyeData *pEyeData)
{
	return 0;
	//if (!pEyeData)
	//	return 0;
	//if (!pEyeData->pData)
	//	return 0;

	//D3D11_MAPPED_SUBRESOURCE mappedResource;

	//int width = 0, height = 0, srcStride = 0, dstStride = 0;
	//int fmt = 0;

	//if (SUCCEEDED(m_pContext->Map(pEyeData->pData->pCPUResource, 0, D3D11_MAP_READ, 0, &mappedResource)))
	//{
	//	pEyeData->pData->pCPUTexture->GetDesc(&pEyeData->Desc);
	//	width = pEyeData->Desc.Width;
	//	height = pEyeData->Desc.Height;
	//	srcStride = mappedResource.RowPitch;
	//	dstStride = srcStride * 2;

	//	//copy eye to pixel buffer. always assume 32 bit pixel				
	//	auto srcPos = (unsigned char *)mappedResource.pData;
	//	auto dstPos = m_DirectScreen.pPixelBuffer[0] + (pEyeData->Eye == EVREye::Eye_Right ? srcStride : 0);

	//	//copy left eye to left, right eye to right
	//	for (unsigned int y = 0; y < pEyeData->Desc.Height; y++)
	//	{
	//		memcpy(dstPos, srcPos, srcStride);
	//		dstPos += dstStride;
	//		srcPos += srcStride;
	//	}
	//	m_pContext->Unmap(pEyeData->pData->pCPUResource, 0);
	//}
	//pEyeData->pData = nullptr;
	//return srcStride;
}

void DirectModeStreamer::ProcessRemotePacket(USBPacket *pPacket)
{
	pHMD->m_pServer->ProcessUSBPacket(pPacket);
}

//
//int DirectStreamer::ProcessFrame(int pitch)
//{
//	return 0;
//	FrameCount++;
//
//	/*AVPixelFormat fmt = AV_PIX_FMT_NONE;
//	if (Left.Desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
//		fmt = AV_PIX_FMT_RGBA;
//	else if (Left.Desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
//		fmt = AV_PIX_FMT_BGRA;
//	else
//	{
//		Sleep(1);
//		return -1;
//	}
//
//	SaveFrameRGB(pPixelBuffer[0], pitch * Left.Desc.Height, FrameCount);
//
//	in_linesize[0] = pitch;
//	pSwsContext = sws_getCachedContext(pSwsContext, Left.Desc.Width * 2, Left.Desc.Height, fmt, Width, Height, STREAM_PIX_FMT, 0, 0, 0, 0);
//	int h = sws_scale(pSwsContext, (const uint8_t * const *)&pPixelBuffer, in_linesize, 0, Height, pFrame->data, pFrame->linesize);
//
//	SaveFrameYUV(pFrame, FrameCount);
//
//	pFrame->pts = FrameCount;
//	pFrame->pkt_duration = (int64_t)FrameTime;
//		
//	return avcodec_send_frame(pCodecContext, pFrame);*/
//}
//
//AVPacket *DirectStreamer::GetPacket()
//{
//	av_init_packet(pPacket);
//	return avcodec_receive_packet(pCodecContext, pPacket) == 0 ? pPacket : nullptr;
//}
//
//bool DirectStreamer::Init(int width, int height, float fps, char *url)
//{
//	ZeroMemory(in_linesize, sizeof(in_linesize));
//	in_linesize[0] = { 4 * width };
//
//	if (fps > 0)
//		FrameTime = 1000.0f / fps;
//	else
//		FrameTime = 100; //default to 10 fps for now
//
//	FPS = (int)(1000 / FrameTime);
//
//	Width = width;
//	Height = height;
//
//	av_register_all();
//	avcodec_register_all();
//
//	Left.Eye = EVREye::Eye_Left;
//	Right.Eye = EVREye::Eye_Right;
//
//	pCodec = avcodec_find_encoder(VIDEO_CODEC_ID);
//	if (!pCodec)
//	{
//		Destroy();
//		return false;
//	}
//
//
//	pCodecContext = avcodec_alloc_context3(pCodec);
//	if (!pCodecContext)
//	{
//		Destroy();
//		return false;
//	}
//
//	AVDictionary *pOptions = nullptr;
//	av_dict_set(&pOptions, "profile", "high", 0);
//	av_dict_set(&pOptions, "preset", "ultrafast", 0);
//	av_dict_set(&pOptions, "tune", "zerolatency", 0);
//	//av_dict_set(&pOptions, "tune", "zerolatency", 0);
//
//	//av_opt_set(&pCodecContext, "profile", "baseline", 0);
//
//	SYSTEM_INFO sysinfo;
//	GetSystemInfo(&sysinfo);
//	int numCPU = max(sysinfo.dwNumberOfProcessors / 2, 1);
//	pCodecContext->profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;
//	pCodecContext->codec_id = VIDEO_CODEC_ID;
//	pCodecContext->bit_rate = Width * Height;
//	pCodecContext->rc_buffer_size = 3;
//	pCodecContext->width = Width;
//	pCodecContext->height = Height;
//	pCodecContext->gop_size = 1;
//	pCodecContext->keyint_min = 1;
//	pCodecContext->pix_fmt = STREAM_PIX_FMT;
//	pCodecContext->time_base.den = FPS;
//	pCodecContext->time_base.num = 1;
//	pCodecContext->max_b_frames = 0;
//
//	pCodecContext->qmin = 5;
//	pCodecContext->qmax = 30;
//	pCodecContext->qcompress = 0;	
//	pCodecContext->qblur = 1;	
//	//pCodecContext->refs = 1;
//	pCodecContext->delay = 0;
//	//pCodecContext->max_qdiff = 10;
//	pCodecContext->slices = 8;
//	pCodecContext->ticks_per_frame = 2;
//
//	pCodecContext->thread_count = numCPU;
//	pCodecContext->thread_type = FF_THREAD_SLICE; // FF_THREAD_SLICE;	
//	pCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//
//
//	//tcp
//
//	//av_opt_set(&pCodecContext, "preset", "ultrafast", 0);
//	//av_opt_set(&pCodecContext, "tune", "zerolatency", 0);
//
//	//av_opt_set(pCodecContext->priv_data, "x264opts", "sync-lookahead=0:slice-max-size=1400", 0);
//
//	if (avcodec_open2(pCodecContext, nullptr, &pOptions) < 0)
//	{
//		Destroy();
//		return false;
//	}
//
//	if (pOptions)
//		av_dict_free(&pOptions);
//
//
//	pPacket = av_packet_alloc();
//	if (!pPacket)
//	{
//		Destroy();
//		return false;
//	}
//
//	pFrame = av_frame_alloc();
//	pFrame->format = STREAM_PIX_FMT;
//	pFrame->width = pCodecContext->width;
//	pFrame->height = pCodecContext->height;
//	if (av_image_alloc(pFrame->data, pFrame->linesize, pFrame->width, pFrame->height, pCodecContext->pix_fmt, 32) < 0)
//	{
//		Destroy();
//		return false;
//	}
//
//	int BufferSize = 3840 * 2160 * 4; //rgba	4k
//	pPixelBuffer[0] = (uint8_t *)malloc(BufferSize);
//	pPixelBuffer[1] = nullptr;
//	pPixelBuffer[2] = nullptr;
//
//	if (pPixelBuffer[0] == nullptr)
//	{
//		Destroy();
//		return false;
//	}
//	ZeroMemory(pPixelBuffer[0], BufferSize);
//
//
//	//InitMFT(Width, Height, FPS);
//	return true;
//}
//
//void DirectStreamer::Destroy()
//{
//	if (pFrame)
//	{
//		if (pFrame->data[0]) av_freep(&pFrame->data[0]);
//		av_frame_free(&pFrame);
//	}
//
//	if (pPacket)
//	{
//		av_packet_unref(pPacket);
//		av_packet_free(&pPacket);
//	}
//
//
//	if (pCodecContext) avcodec_close(pCodecContext);
//
//	pCodecContext = nullptr;
//	pCodec = nullptr;
//
//	SAFE_FREE(pPixelBuffer[0]);
//}
//
//void DirectStreamer::SaveFrameYUV(AVFrame *pFrame, int frameno)
//{
//	return;
//	FILE *yuvFile = nullptr;
//	char szFilename[MAX_PATH];
//	sprintf_s(szFilename, "D:\\OUT\\xx-%d.yuv", frameno);
//	fopen_s(&yuvFile, szFilename, "wb");
//	if (yuvFile)
//	{
//		fwrite(pFrame->data[0], 1, pCodecContext->width * pCodecContext->height, yuvFile);
//		fwrite(pFrame->data[1], 1, pCodecContext->width * pCodecContext->height / 4, yuvFile);
//		fwrite(pFrame->data[2], 1, pCodecContext->width * pCodecContext->height / 4, yuvFile);
//		fclose(yuvFile);
//	}
//}
//
//void DirectStreamer::SaveFrameRGB(uint8_t *pixels, int size, int frameno)
//{
//	return;		
//	FILE *yuvFile = nullptr;
//	char szFilename[MAX_PATH];
//	sprintf_s(szFilename, "D:\\OUT\\xx-%d.rgb", frameno);
//	fopen_s(&yuvFile, szFilename, "wb");
//	if (yuvFile)
//	{
//		fwrite(pixels, 1, size, yuvFile);
//		fclose(yuvFile);
//	}
//}

#define EXIT_IF_FAILED_MFT if (FAILED(hr)) { DestroyMFT(); return false; }

bool DirectModeStreamer::InitMFT()
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

	fopen_s(&m_FileDump, "D:\\OUT\\xx.h264", "wb");
	

	hr = m_pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0); EXIT_IF_FAILED_MFT;
	hr = m_pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0); EXIT_IF_FAILED_MFT;
	hr = m_pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0); EXIT_IF_FAILED_MFT;

	//encoder ready

	return true;
}

void DirectModeStreamer::DestroyMFT()
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
	if (m_FileDump)
		fclose(m_FileDump);
	m_FileDump = nullptr;
}

IMFMediaEvent *DirectModeStreamer::GetEvent()
{
	if (!m_pEventGenerator) return nullptr;
	IMFMediaEvent *pEvent = nullptr;
	if (SUCCEEDED(m_pEventGenerator->GetEvent(MF_EVENT_FLAG_NO_WAIT, &pEvent)))
		return pEvent;
	return nullptr;
}


/*















*/

HRESULT CMFMediaBufferWrapper::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == __uuidof(IMFMediaBuffer))
	{
		*ppvObject = (void *)this;
		return S_OK;
	}
	if (riid == __uuidof(IUnknown))
	{
		*ppvObject = (void *)(IUnknown*)this;
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG CMFMediaBufferWrapper::AddRef(void)
{
	return ++m_RefCount;
}

ULONG CMFMediaBufferWrapper::Release(void)
{
	m_RefCount--;
	if (!m_RefCount)
		delete this;
	return m_RefCount;
}

HRESULT CMFMediaBufferWrapper::Lock(BYTE ** ppbBuffer, DWORD * pcbMaxLength, DWORD * pcbCurrentLength)
{
	return S_OK;
}

HRESULT CMFMediaBufferWrapper::Unlock(void)
{
	return S_OK;
}

HRESULT CMFMediaBufferWrapper::GetCurrentLength(DWORD * pcbCurrentLength)
{
	*pcbCurrentLength = m_CurrLength;
	return S_OK;
}

HRESULT CMFMediaBufferWrapper::SetCurrentLength(DWORD cbCurrentLength)
{
	m_CurrLength = cbCurrentLength;
	return S_OK;
}

HRESULT CMFMediaBufferWrapper::GetMaxLength(DWORD * pcbMaxLength)
{
	*pcbMaxLength = m_MaxLength;
	return S_OK;
}
