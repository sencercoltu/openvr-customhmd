#include "DirectModeOutput.h"
#include "TrackedHMD.h"

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "Dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

extern HMODULE g_hModule;

#define EXIT_IF_FAILED if (FAILED(hr)) { Destroy(); return; }

LRESULT CALLBACK DirectModeOutput::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;	 
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void DirectModeOutput::Init(CTrackedHMD *pHmd)
{
	pHMD = pHmd;

	m_LeftRotate = pHmd->m_HMDData.LeftRotate;
	m_RightRotate = pHmd->m_HMDData.RightRotate;

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	m_hTextureMapLock = CreateMutex(nullptr, FALSE, L"TextureMapLock");

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_11_1
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
	float ROTATE; \
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
	output.TexCoord.x = ((inTexCoord.x - 0.5) * cos(ROTATE)) - ((inTexCoord.y - 0.5) * sin(ROTATE)) + 0.5; \
	output.TexCoord.y = ((inTexCoord.x - 0.5) * sin(ROTATE)) + ((inTexCoord.y -0.5) * cos(ROTATE)) + 0.5; \
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

}

void DirectModeOutput::Destroy()
{
	m_FrameTime = 0;
	m_IsRunning = false;
	if (m_hWnd) DestroyWindow(m_hWnd);
	
	SAFE_RELEASE(m_pContext);
	SAFE_RELEASE(m_pDevice);

	SAFE_RELEASE(m_pSyncTexture);
	SAFE_CLOSE(m_hTextureMapLock);

	SAFE_RELEASE(m_pRTView);
	//SAFE_RELEASE(m_pRTTex);
	SAFE_RELEASE(m_pSwapChain);

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

	CoUninitialize();
	m_hWnd = nullptr;
	pHMD = nullptr;
}


void DirectModeOutput::CreateOutputWindow()
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC;  CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = g_hModule;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszClassName = L"DirectOutputWindow";
	RegisterClassEx(&wc);

	// create the window and use the result as the handle
	m_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME,
		L"DirectOutputWindow",    // name of the window class
		L"Direct Mode Render Output",   // title of the window
		WS_VISIBLE | WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,    // window style
		0,    // x-position of the window
		0,    // y-position of the window
		pHMD->m_HMDData.ScreenWidth,    // width of the window
		pHMD->m_HMDData.ScreenHeight,    // height of the window
		nullptr,    // we have no parent window, NULL
		nullptr,    // we aren't using menus, NULL
		nullptr,    // application handle
		nullptr);    // used with multiple windows, NULL

	if (!m_hWnd)
		return;

	IDXGIFactory* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	EXIT_IF_FAILED;

	DXGI_MODE_DESC bufferDesc = {};
	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
	bufferDesc.Width = pHMD->m_HMDData.ScreenWidth;
	bufferDesc.Height = pHMD->m_HMDData.ScreenHeight;
	bufferDesc.RefreshRate.Numerator = (UINT)pHMD->m_HMDData.Frequency;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;


	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	hr = dxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	dxgiFactory->Release();
	EXIT_IF_FAILED;

	ID3D11Texture2D* pBackBuffer;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	EXIT_IF_FAILED;

	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRTView);
	pBackBuffer->Release();
	EXIT_IF_FAILED;

	//MessageBox(nullptr, L"OK", L"OK", 0);
	//ShowWindow(m_hWnd, SW_SHOW);

}

void DirectModeOutput::RenderOutput()
{
	if (!m_hWnd)
	{
		CreateOutputWindow();
		OutputDebugString(L"Created window\n");
	}

	if (!m_hWnd || !m_pSwapChain)
	{
		OutputDebugString(L"No window\n");
		return;
	}

	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	

	DWORD now = GetTickCount(); //limit framerate to display
	if (now - m_LastFrameTime < m_FrameTime)
		return;
	m_LastFrameTime = now;

	ID3D11ShaderResourceView *pEmptyTexture[1] = { nullptr };
	m_pContext->OMSetRenderTargets(1, &m_pRTView, nullptr);

	//auto test rotate
	//m_LeftRotate -= 0.1f;
	//m_RightRotate += 0.1f;

	//draw left texture to combined texture
	if (m_TlLeft.pData)
	{
		Ep.SHIFT = XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, 0.0f));
		Ep.ROTATE = XMConvertToRadians(m_LeftRotate);
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
		Ep.ROTATE = XMConvertToRadians(m_RightRotate);
		m_pContext->PSSetShaderResources(0, 1, &m_TlRight.pData->pShaderResourceView);
		m_pContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &Ep, 0, 0);
		m_pContext->DrawIndexed(6, 0, 0);
		m_pContext->PSSetShaderResources(0, 1, pEmptyTexture);
		m_TlRight.pData = nullptr;
	}

	m_pSwapChain->Present(0, 0);
}

