#include "D3D11Renderer.h"
#include "D3DIncludes.h"
#include "EffectsManager.h"
#include "MeshRenderer.h"

D3D11Renderer* D3D11Renderer::_instance = nullptr;

D3D11Renderer::D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd):
m_hInst(hInstance),
m_hMainWnd(hMainWnd),
m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
m_clientWidth(1280),
m_clientHeight(720),
m_enable4xMsaa(false),
m_4xMsaaQuality(0),

m_d3d11Device(nullptr),
m_d3d11DeviceContext(nullptr),
m_swapChain(nullptr),
m_depthStencilBuffer(nullptr),
m_renderTargetView(nullptr),
m_depthStencilView(nullptr)
{
	ZeroMemory(&m_screenViewpot, sizeof(D3D11_VIEWPORT));
	_instance = this;
}

D3D11Renderer::~D3D11Renderer() {
	ReleaseCOM(m_renderTargetView);
	ReleaseCOM(m_depthStencilView);
	ReleaseCOM(m_swapChain);
	ReleaseCOM(m_depthStencilBuffer);

	if (m_d3d11DeviceContext)
		m_d3d11DeviceContext->ClearState();

	ReleaseCOM(m_d3d11DeviceContext);
	ReleaseCOM(m_d3d11Device);
}

bool D3D11Renderer::Init() {
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
				 0,
				 m_d3dDriverType,
				 0,
				 createDeviceFlags,
				 0,
				 0,
				 D3D11_SDK_VERSION,
				 &m_d3d11Device,
				 &featureLevel,
				 &m_d3d11DeviceContext);

	if (FAILED(hr))	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	HR(m_d3d11Device->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));
	assert(m_4xMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_clientWidth;
	sd.BufferDesc.Height = m_clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (m_enable4xMsaa) {
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else {
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = m_hMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	IDXGIDevice* dxgiDevice = 0;
	HR(m_d3d11Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(m_d3d11Device, &sd, &m_swapChain));

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	ID3D11Texture2D* backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_d3d11Device->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	ReleaseCOM(backBuffer);

	// --------

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_clientWidth;
	depthStencilDesc.Height = m_clientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (m_enable4xMsaa)	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	// No MSAA
	else {
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_d3d11Device->CreateTexture2D(&depthStencilDesc, 0, &m_depthStencilBuffer));
	HR(m_d3d11Device->CreateDepthStencilView(m_depthStencilBuffer, 0, &m_depthStencilView));

	m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	m_screenViewpot.TopLeftX = 0;
	m_screenViewpot.TopLeftY = 0;
	m_screenViewpot.Width = static_cast<float>(m_clientWidth);
	m_screenViewpot.Height = static_cast<float>(m_clientHeight);
	m_screenViewpot.MinDepth = 0.0f;
	m_screenViewpot.MaxDepth = 1.0f;

	m_d3d11DeviceContext->RSSetViewports(1, &m_screenViewpot);

	mesh = new MeshRenderer();

	return true;
}

void D3D11Renderer::Draw() {
	m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


 		PosColorEffect* activeFX = (PosColorEffect*)(EffectsManager::Instance()->m_effects[0]);
		D3D11Renderer::Instance()->GetD3DContext()->IASetInputLayout(activeFX->GetInputLayout());
		D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		XMFLOAT4X4 mWorld;
		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;

		XMMATRIX I = XMMatrixIdentity();
		XMStoreFloat4x4(&mWorld, I);
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);

		XMVECTOR pos = XMVectorSet(0, 3.5, -3.5, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, V);

		// The window resized, so update the aspect ratio and recompute the projection matrix.
		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*3.1415926, 16.0/9.0, 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, P);

		// Set constants
		XMMATRIX world = XMLoadFloat4x4(&mWorld);
		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX worldViewProj = world*view*proj;

		activeFX->WorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

		for (int iMesh = 0; iMesh < activeFX->m_associatedMeshes.size(); iMesh++ ) {
			activeFX->m_associatedMeshes[iMesh]->Draw();
		}

		//mesh->Draw(activeFX->PosColorTech);
			
	

	HR(m_swapChain->Present(0, 0));
}
