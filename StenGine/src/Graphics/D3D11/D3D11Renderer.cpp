#if GRAPHICS_D3D11

#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Color.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/Terrain.h"
#include "Scene/LightManager.h"
#include "Scene/CameraManager.h"
#include "Math/MathHelper.h"

#pragma warning(disable: 4267 4244)

#define FORWARD 0

class D3D11Renderer : public Renderer
{
public:
	D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd) :
		m_hInst(hInstance),
		m_hMainWnd(hMainWnd),
		m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
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
		m_clientWidth = 1280;
		m_clientHeight = 720;
	}

	~D3D11Renderer() {

		ReleaseCOM(m_renderTargetView);
		ReleaseCOM(m_depthStencilView);
		ReleaseCOM(m_swapChain);
		ReleaseCOM(m_depthStencilBuffer);

		if (m_d3d11DeviceContext)
			m_d3d11DeviceContext->ClearState();

		ReleaseCOM(m_d3d11DeviceContext);
		ReleaseCOM(m_d3d11Device);
		SafeDelete(m_SkyBox);
	}

	void Release() override {
		_instance = nullptr;
		delete this;
	}

	bool Init(int32_t width, int32_t height, CreateWindowCallback createWindow) override {

		if (!createWindow(width, height, m_hInst, m_hMainWnd))
		{
			return false;
		}

		SetWindowText(m_hMainWnd, L"StenGine");
		ShowWindow(m_hMainWnd, SW_SHOW);

		SetFocus(m_hMainWnd);

		UINT createDeviceFlags = 0;
#if (defined(DEBUG) || defined(_DEBUG))  
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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

		if (FAILED(hr)) {
			MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
			return false;
		}

		if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
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

		if (m_enable4xMsaa) {
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


		m_screenViewpot.TopLeftX = 0;
		m_screenViewpot.TopLeftY = 0;
		m_screenViewpot.Width = static_cast<float>(m_clientWidth);
		m_screenViewpot.Height = static_cast<float>(m_clientHeight);
		m_screenViewpot.MinDepth = 0.0f;
		m_screenViewpot.MaxDepth = 1.0f;

		m_screenSuperSampleViewpot.TopLeftX = 0;
		m_screenSuperSampleViewpot.TopLeftY = 0;
		m_screenSuperSampleViewpot.Width = static_cast<float>(m_clientWidth);
		m_screenSuperSampleViewpot.Height = static_cast<float>(m_clientHeight);
		m_screenSuperSampleViewpot.MinDepth = 0.0f;
		m_screenSuperSampleViewpot.MaxDepth = 1.0f;

		{
			D3D11_SAMPLER_DESC samplerDesc;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[0] = 0;
			samplerDesc.BorderColor[1] = 0;
			samplerDesc.BorderColor[2] = 0;
			samplerDesc.BorderColor[3] = 0;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			// Create the texture sampler state.
			hr = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_samplerState);
			assert(SUCCEEDED(hr));
		}

		{
			D3D11_SAMPLER_DESC samplerDesc;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[0] = 0;
			samplerDesc.BorderColor[1] = 0;
			samplerDesc.BorderColor[2] = 0;
			samplerDesc.BorderColor[3] = 0;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			// Create the texture sampler state.
			hr = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_heightMapSamplerState);
			assert(SUCCEEDED(hr));
		}

		{
			D3D11_SAMPLER_DESC shadowSamplerDesc;
			ZeroMemory(&shadowSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
			shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			shadowSamplerDesc.BorderColor[0] = 1;
			shadowSamplerDesc.BorderColor[1] = 1;
			shadowSamplerDesc.BorderColor[2] = 1;
			shadowSamplerDesc.BorderColor[3] = 1;
			shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

			// Create the texture shadowSampler state.
			hr = m_d3d11Device->CreateSamplerState(&shadowSamplerDesc, &m_shadowSamplerState);
			assert(SUCCEEDED(hr));
		}

		{
			D3D11_SAMPLER_DESC samplerDesc;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[0] = 0;
			samplerDesc.BorderColor[1] = 0;
			samplerDesc.BorderColor[2] = 0;
			samplerDesc.BorderColor[3] = 0;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			// Create the texture sampler state.
			hr = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_borderSamplerState);
			assert(SUCCEEDED(hr));
		}

		//-----------------setup MRT---------------------
#if !FORWARD
		D3D11_TEXTURE2D_DESC gNormalBufferDesc;
		gNormalBufferDesc.Width = m_clientWidth;
		gNormalBufferDesc.Height = m_clientHeight;
		gNormalBufferDesc.MipLevels = 1;
		gNormalBufferDesc.ArraySize = 1;
		gNormalBufferDesc.SampleDesc.Count = 1;
		gNormalBufferDesc.SampleDesc.Quality = 0;
		gNormalBufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		gNormalBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		gNormalBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		gNormalBufferDesc.CPUAccessFlags = 0;
		gNormalBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;



		D3D11_TEXTURE2D_DESC gBufferDesc;
		gBufferDesc.Width = m_clientWidth;
		gBufferDesc.Height = m_clientHeight;
		gBufferDesc.MipLevels = 1;
		gBufferDesc.ArraySize = 1;
		gBufferDesc.SampleDesc.Count = 1;
		gBufferDesc.SampleDesc.Quality = 0;
		gBufferDesc.Format = /*DXGI_FORMAT_R8G8B8A8_UNORM*/DXGI_FORMAT_R16G16B16A16_FLOAT;
		gBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		gBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		gBufferDesc.CPUAccessFlags = 0;
		gBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		ID3D11Texture2D* gBufferDTex = 0;
		ID3D11Texture2D* gBufferNTex = 0;
		ID3D11Texture2D* gBufferSTex = 0;
		ID3D11Texture2D* gBufferPTex = 0;
		ID3D11Texture2D* gSSAOTex = 0;
		ID3D11Texture2D* gSSAOTex2 = 0;
		ID3D11Texture2D* gDeferredShadeTex = 0;
		//ID3D11Texture2D* gBufferETex = 0;
		HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferDTex));
		HR(m_d3d11Device->CreateTexture2D(&gNormalBufferDesc, 0, &gBufferNTex));
		HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferSTex));
		HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferPTex));
		HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gSSAOTex));
		HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gSSAOTex2));
		HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gDeferredShadeTex));
		//HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferETex));

		D3D11_RENDER_TARGET_VIEW_DESC rtvNormalDesc;
		rtvNormalDesc.Format = gNormalBufferDesc.Format;
		rtvNormalDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvNormalDesc.Texture2D.MipSlice = 0;


		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = gBufferDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		HR(m_d3d11Device->CreateRenderTargetView(gBufferDTex, &rtvDesc, &m_diffuseBufferRTV));
		HR(m_d3d11Device->CreateRenderTargetView(gBufferNTex, &rtvNormalDesc, &m_normalBufferRTV));
		HR(m_d3d11Device->CreateRenderTargetView(gBufferSTex, &rtvDesc, &m_specularBufferRTV));
		HR(m_d3d11Device->CreateRenderTargetView(gBufferPTex, &rtvDesc, &m_positionBufferRTV));
		HR(m_d3d11Device->CreateRenderTargetView(gSSAOTex, &rtvDesc, &m_SSAORTV));
		HR(m_d3d11Device->CreateRenderTargetView(gSSAOTex2, &rtvDesc, &m_SSAORTV2));
		HR(m_d3d11Device->CreateRenderTargetView(gDeferredShadeTex, &rtvDesc, &m_deferredShadingRTV));
		//HR(m_d3d11Device->CreateRenderTargetView(gBufferETex, &rtvDesc, &m_edgeBufferRTV));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvNormalDesc;
		srvNormalDesc.Format = gNormalBufferDesc.Format;
		srvNormalDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvNormalDesc.Texture2D.MostDetailedMip = 0;
		srvNormalDesc.Texture2D.MipLevels = -1;


		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = gBufferDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		HR(m_d3d11Device->CreateShaderResourceView(gBufferDTex, &srvDesc, &m_diffuseBufferSRV));
		HR(m_d3d11Device->CreateShaderResourceView(gBufferNTex, &srvNormalDesc, &m_normalBufferSRV));
		HR(m_d3d11Device->CreateShaderResourceView(gBufferSTex, &srvDesc, &m_specularBufferSRV));
		HR(m_d3d11Device->CreateShaderResourceView(gBufferPTex, &srvDesc, &m_positionBufferSRV));
		HR(m_d3d11Device->CreateShaderResourceView(gSSAOTex, &srvDesc, &m_SSAOSRV));
		HR(m_d3d11Device->CreateShaderResourceView(gSSAOTex2, &srvDesc, &m_SSAOSRV2));
		HR(m_d3d11Device->CreateShaderResourceView(gDeferredShadeTex, &srvDesc, &m_deferredShadingSRV));
		//HR(m_d3d11Device->CreateShaderResourceView(gBufferETex, &srvDesc, &m_edgeBufferSRV));

		ReleaseCOM(gBufferDTex);
		ReleaseCOM(gBufferNTex);
		ReleaseCOM(gBufferSTex);
		ReleaseCOM(gBufferPTex);
		ReleaseCOM(gSSAOTex);
		ReleaseCOM(gDeferredShadeTex);

		D3D11_TEXTURE2D_DESC depthTexDesc;
		depthTexDesc.Width = m_clientWidth;
		depthTexDesc.Height = m_clientHeight;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = 0;

		ID3D11Texture2D* depthTex = 0;
		HR(m_d3d11Device->CreateTexture2D(&depthTexDesc, 0, &depthTex));

		// Create the depth stencil view for the entire cube
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Flags = 0;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		HR(m_d3d11Device->CreateDepthStencilView(depthTex, &dsvDesc, &m_deferredRenderDepthStencilView));

		D3D11_SHADER_RESOURCE_VIEW_DESC dsrvDesc;
		dsrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		dsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dsrvDesc.Texture2D.MipLevels = depthTexDesc.MipLevels;
		dsrvDesc.Texture2D.MostDetailedMip = 0;
		HR(m_d3d11Device->CreateShaderResourceView(depthTex, &dsrvDesc, &m_deferredRenderShaderResourceView));

		ReleaseCOM(depthTex);

#endif

		D3D11_RASTERIZER_DESC wireframeDesc;
		ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
		wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
		wireframeDesc.CullMode = D3D11_CULL_BACK;
		wireframeDesc.FrontCounterClockwise = false;
		wireframeDesc.DepthClipEnable = true;

		HR(m_d3d11Device->CreateRasterizerState(&wireframeDesc, &m_wireFrameRS));

		D3D11_RASTERIZER_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(D3D11_RASTERIZER_DESC));
		depthDesc.FillMode = D3D11_FILL_SOLID;
		depthDesc.CullMode = D3D11_CULL_BACK;
		depthDesc.DepthBias = 100000;
		depthDesc.FrontCounterClockwise = false;
		depthDesc.DepthClipEnable = true;
		depthDesc.DepthBiasClamp = 0.0f;
		depthDesc.SlopeScaledDepthBias = 1.0f;

		HR(m_d3d11Device->CreateRasterizerState(&depthDesc, &m_depthRS));


		DirectionalLight* dLight = new DirectionalLight();
		dLight->intensity = XMFLOAT4(1, 1, 1, 1);
		dLight->direction = MatrixHelper::NormalizeFloat3(XMFLOAT3(-0.5, -2, 1));
		dLight->castShadow = 1;

		LightManager::Instance()->m_dirLights.push_back(dLight);
		LightManager::Instance()->m_shadowMap = new ShadowMap(2048, 2048);

		m_SkyBox = new Skybox(std::wstring(L"Model/sunsetcube1024.dds"));

		m_d3d11DeviceContext->RSSetState(m_wireFrameRS);

		CreateDDSTextureFromFile(m_d3d11Device,
			L"./Model/RandNorm.dds", nullptr, &m_randVecTexSRV);

		// init grid and coord debug draw
		std::vector<XMFLOAT3> coordVertexBuffer = {
			XMFLOAT3(0, 0, 0),
			XMFLOAT3(5, 0, 0),
			XMFLOAT3(0, 0, 0),
			XMFLOAT3(0, 5, 0),
			XMFLOAT3(0, 0, 0),
			XMFLOAT3(0, 0, 5),
		};

		std::vector<UINT> coordIndexBuffer = { 0, 1, 2, 3, 4, 5 };


		int initIdx = 6;
		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.push_back(XMFLOAT3(-5.f, 0.f, -5.f + i));
			coordVertexBuffer.push_back(XMFLOAT3(5.f, 0.f, -5.f + i));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.push_back(XMFLOAT3(-5.f + i, 0.f, -5.f));
			coordVertexBuffer.push_back(XMFLOAT3(-5.f + i, 0.f, 5.f));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex::StdMeshVertex) * coordVertexBuffer.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		//vbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &coordVertexBuffer[0];
		HR(m_d3d11Device->CreateBuffer(&vbd, &vinitData, &m_gridCoordVertexBufferGPU));

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * coordIndexBuffer.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &coordIndexBuffer[0];
		HR(m_d3d11Device->CreateBuffer(&ibd, &iinitData, &m_gridCoordIndexBufferGPU));

		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			// Depth test parameters
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

			// Stencil test parameters
			dsDesc.StencilEnable = true;
			dsDesc.StencilReadMask = 0xFF;
			dsDesc.StencilWriteMask = 0xFF;

			// Stencil operations if pixel is front-facing
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Stencil operations if pixel is back-facing
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			m_d3d11Device->CreateDepthStencilState(&dsDesc, &m_noZWriteDSState);
		}

		{
			D3D11_BLEND_DESC bsDesc;
			bsDesc.AlphaToCoverageEnable = FALSE;
			bsDesc.IndependentBlendEnable = FALSE;

			bsDesc.RenderTarget[0].BlendEnable = TRUE;
			bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
			bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			HRESULT hr = m_d3d11Device->CreateBlendState(&bsDesc, &m_additiveAlphaAddBS);
			assert(SUCCEEDED(hr));
		}

		return true;
	}

	void D3D11Renderer::Draw() override {

		ID3D11SamplerState* samplerState[] = { m_samplerState, m_shadowSamplerState, m_heightMapSamplerState };
		m_d3d11DeviceContext->PSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->VSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->DSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->HSSetSamplers(0, 3, samplerState);

		DrawShadowMap();
		DrawGBuffer();
		DrawDeferredShading();
		DrawBlurSSAOAndCombine();
		//DrawGodRay();
		DrawDebug();

		// clean up
		m_d3d11DeviceContext->ClearState();
		m_d3d11DeviceContext->RSSetState(0);
		m_d3d11DeviceContext->OMSetDepthStencilState(0, 0);
		m_d3d11DeviceContext->OMSetRenderTargets(0, NULL, NULL);
		HR(m_swapChain->Present(0, 0));
	}

	float GetAspectRatio() override {
		return static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight);
	}

	int GetScreenWidth() override {
		return m_clientWidth;
	}

	int GetScreenHeight() override {
		return m_clientHeight;
	}

	Skybox* GetSkyBox() override {
		return m_SkyBox;
	}

	void* GetDepthRS() override {
		return m_depthRS;
	}

	void D3D11Renderer::DrawGBuffer() {
		m_d3d11DeviceContext->RSSetViewports(1, &m_screenViewpot);
		m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D11RenderTargetView* rtvs[3] = { m_diffuseBufferRTV, m_normalBufferRTV, m_specularBufferRTV };
		m_d3d11DeviceContext->OMSetRenderTargets(3, rtvs, m_deferredRenderDepthStencilView);
		m_d3d11DeviceContext->ClearRenderTargetView(m_diffuseBufferRTV, reinterpret_cast<const float*>(&SGColors::LightSteelBlue));
		m_d3d11DeviceContext->ClearRenderTargetView(m_positionBufferRTV, reinterpret_cast<const float*>(&SGColors::White));
		m_d3d11DeviceContext->ClearRenderTargetView(m_normalBufferRTV, reinterpret_cast<const float*>(&SGColors::Black));
		m_d3d11DeviceContext->ClearRenderTargetView(m_specularBufferRTV, reinterpret_cast<const float*>(&SGColors::Black));
		m_d3d11DeviceContext->ClearDepthStencilView(m_deferredRenderDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_d3d11DeviceContext->RSSetState(0);
		m_d3d11DeviceContext->IASetInputLayout((ID3D11InputLayout *)EffectsManager::Instance()->m_deferredGeometryPassEffect->GetInputLayout());

		XMFLOAT4 pos = CameraManager::Instance()->GetActiveCamera()->GetPos();

		ID3D11SamplerState* samplerState[] = { m_samplerState, m_shadowSamplerState, m_heightMapSamplerState };
		m_d3d11DeviceContext->PSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->VSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->DSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->HSSetSamplers(0, 3, samplerState);

		for (uint32_t iMesh = 0; iMesh < EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes.size(); iMesh++) {
			EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes[iMesh]->GatherDrawCall();
		}

		for (auto &cmd : m_deferredDrawList)
		{
			cmd.effect->SetShader();
			m_d3d11DeviceContext->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)cmd.type);

			//if (m_currentVao != (uint64_t)cmd.m_vertexArrayObject)
			//{
			//	m_currentVao = (uint64_t)cmd.m_vertexArrayObject;
			//	glBindVertexArray((uint64_t)cmd.m_vertexArrayObject);
			//}
			ID3D11Buffer* vertexBuffer = static_cast<ID3D11Buffer*>(cmd.vertexBuffer);
			ID3D11Buffer* indexBuffer = static_cast<ID3D11Buffer*>(cmd.indexBuffer);

			m_d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &cmd.vertexStride, &cmd.vertexOffset);
			m_d3d11DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			for (auto &cbuffer : cmd.cbuffers)
			{
				cbuffer.Bind();
			}

			cmd.srvs.Bind();

			m_d3d11DeviceContext->DrawIndexed(cmd.elementCount, (int64_t)cmd.offset, 0);

			cmd.srvs.Unbind();
		}

		m_deferredDrawList.clear();

		Terrain::Instance()->Draw();
	}

	void D3D11Renderer::DrawDeferredShading() {
		ID3D11ShaderResourceView* nullSRV[16] = { 0 };
		// --------- skybox --------- //
		m_d3d11DeviceContext->PSSetShaderResources(0, 16, nullSRV);
		m_d3d11DeviceContext->HSSetShaderResources(0, 16, nullSRV);
		m_d3d11DeviceContext->DSSetShaderResources(0, 16, nullSRV);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &m_deferredShadingRTV, m_depthStencilView);
		m_d3d11DeviceContext->ClearRenderTargetView(m_deferredShadingRTV, reinterpret_cast<const float*>(&SGColors::LightSteelBlue));
		m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_SkyBox->Draw();

		m_d3d11DeviceContext->RSSetState(0);
		m_d3d11DeviceContext->OMSetDepthStencilState(0, 0);
		m_d3d11DeviceContext->OMSetRenderTargets(0, NULL, NULL);

#define PS_SHADING 1
#if PS_SHADING 
		//-------------- composite deferred render target views AND SSAO-------------------//

		m_d3d11DeviceContext->RSSetViewports(1, &m_screenViewpot);
		ID3D11RenderTargetView* crtvs[2] = { m_deferredShadingRTV, m_SSAORTV };
		m_d3d11DeviceContext->OMSetRenderTargets(2, crtvs, m_depthStencilView);
		m_d3d11DeviceContext->ClearRenderTargetView(m_SSAORTV, reinterpret_cast<const float*>(&SGColors::LightSteelBlue));
		m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_d3d11DeviceContext->IASetInputLayout(NULL);

		DeferredShadingPassEffect* deferredShadingEffect = EffectsManager::Instance()->m_deferredShadingPassEffect;
		deferredShadingEffect->SetShader();

		DirectionalLight viewDirLight;
		memcpy(&viewDirLight, LightManager::Instance()->m_dirLights[0], sizeof(DirectionalLight));

		XMMATRIX ViewInvTranspose = MatrixHelper::InverseTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewMatrix());
		XMStoreFloat3(&viewDirLight.direction, XMVector3Transform(XMLoadFloat3(&viewDirLight.direction), ViewInvTranspose));
		deferredShadingEffect->m_perFrameConstantBuffer.gDirLight = viewDirLight;

		XMFLOAT4 camPos = CameraManager::Instance()->GetActiveCamera()->GetPos();
		XMStoreFloat4(&camPos, XMVector3Transform(XMLoadFloat4(&camPos), CameraManager::Instance()->GetActiveCamera()->GetViewMatrix()));
		deferredShadingEffect->m_perFrameConstantBuffer.gEyePosV = camPos;

		XMMATRIX projMat = CameraManager::Instance()->GetActiveCamera()->GetProjMatrix();
		XMVECTOR det = XMMatrixDeterminant(projMat);
		deferredShadingEffect->m_perFrameConstantBuffer.gProj = XMMatrixTranspose(projMat);

		deferredShadingEffect->m_perFrameConstantBuffer.gProjInv = XMMatrixTranspose(XMMatrixInverse(&det, projMat));

		deferredShadingEffect->SetShaderResources(m_diffuseBufferSRV, 0);
		deferredShadingEffect->SetShaderResources(m_normalBufferSRV, 1);
		deferredShadingEffect->SetShaderResources(m_specularBufferSRV, 2);
		deferredShadingEffect->SetShaderResources(m_deferredRenderShaderResourceView, 3);
		deferredShadingEffect->SetShaderResources(m_randVecTexSRV, 4);

		m_d3d11DeviceContext->PSSetSamplers(0, 1, &m_samplerState);

		deferredShadingEffect->UpdateConstantBuffer();
		deferredShadingEffect->BindConstantBuffer();
		deferredShadingEffect->BindShaderResource();
		m_d3d11DeviceContext->Draw(6, 0);
		deferredShadingEffect->UnBindShaderResource();
		deferredShadingEffect->UnBindConstantBuffer();

		deferredShadingEffect->UnSetShader();


#else
		// -------compute shader deferred shading -------//


		m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		DeferredShadingCS* deferredCSEffect = EffectsManager::Instance()->m_deferredShadingCSEffect;
		deferredCSEffect->SetShader();

		DirectionalLight viewDirLight;
		memcpy(&viewDirLight, LightManager::Instance()->m_dirLights[0], sizeof(DirectionalLight));

		XMMATRIX ViewInvTranspose = MatrixHelper::InverseTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewMatrix());
		XMStoreFloat3(&viewDirLight.direction, XMVector3Transform(XMLoadFloat3(&viewDirLight.direction), ViewInvTranspose));
		deferredCSEffect->m_perFrameConstantBuffer.gDirLight = viewDirLight;

		XMFLOAT4 camPos = CameraManager::Instance()->GetActiveCamera()->GetPos();
		XMStoreFloat4(&camPos, XMVector3Transform(XMLoadFloat4(&camPos), CameraManager::Instance()->GetActiveCamera()->GetViewMatrix()));
		deferredCSEffect->m_perFrameConstantBuffer.gEyePosW = camPos;

		XMMATRIX projMat = CameraManager::Instance()->GetActiveCamera()->GetProjMatrix();
		XMVECTOR det = XMMatrixDeterminant(projMat);
		deferredCSEffect->m_perFrameConstantBuffer.gProj = XMMatrixTranspose(projMat);

		deferredCSEffect->m_perFrameConstantBuffer.gProjInv = XMMatrixTranspose(XMMatrixInverse(&det, projMat));

		deferredCSEffect->SetShaderResources(m_diffuseBufferSRV, 0);
		deferredCSEffect->SetShaderResources(m_normalBufferSRV, 1);
		deferredCSEffect->SetShaderResources(m_specularBufferSRV, 2);
		deferredCSEffect->SetShaderResources(m_deferredRenderShaderResourceView, 3);

		m_d3d11DeviceContext->CSSetSamplers(0, 1, &m_samplerState);

		deferredCSEffect->UpdateConstantBuffer();
		deferredCSEffect->BindConstantBuffer();
		deferredCSEffect->BindShaderResource();

		m_d3d11DeviceContext->Dispatch(80, 45, 1);

		deferredCSEffect->UnBindConstantBuffer();
		deferredCSEffect->UnbindUnorderedAccessViews();
		deferredCSEffect->UnBindShaderResource();
		deferredCSEffect->UnSetShader();

#endif
	}

	void D3D11Renderer::DrawBlurSSAOAndCombine() {
		ID3D11ShaderResourceView* nullSRV[16] = { 0 };
		ID3D11SamplerState* samplerState[] = { m_samplerState, m_shadowSamplerState };
		// -------compute shader blur ----------//
		m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

#if PS_SHADING
		ID3D11ShaderResourceView* blurredSSAOSRV = doCSBlur(m_SSAOSRV, 0);
		ID3D11ShaderResourceView* blurredSRV = doCSBlur(m_deferredShadingSRV, 1);
#else
		ID3D11ShaderResourceView* blurredSSAOSRV = doCSBlur(deferredCSEffect->GetOutputShaderResource(1), 0);
		ID3D11ShaderResourceView* blurredSRV = doCSBlur(deferredCSEffect->GetOutputShaderResource(0), 1);
#endif

		// ------ Screen Quad -------//
		BlurEffect* blurEffect = EffectsManager::Instance()->m_blurEffect;
		blurEffect->SetShader();

		m_d3d11DeviceContext->PSSetSamplers(0, 1, samplerState);

		m_d3d11DeviceContext->PSSetShaderResources(0, 16, nullSRV);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#if PS_SHADING
		blurEffect->SetShaderResources(m_deferredShadingSRV, 0);
#else
		blurEffect->SetShaderResources(deferredCSEffect->GetOutputShaderResource(0), 0);
#endif
		blurEffect->SetShaderResources(blurredSSAOSRV/*m_SSAOSRV*/, 1);
		blurEffect->SetShaderResources(blurredSRV, 2);
		blurEffect->m_settingConstantBuffer.texOffset = XMFLOAT2(1.0f / m_clientWidth, 0.0f);
		m_d3d11DeviceContext->PSSetSamplers(0, 1, samplerState);

		blurEffect->UpdateConstantBuffer();
		blurEffect->BindConstantBuffer();
		blurEffect->BindShaderResource();
		m_d3d11DeviceContext->Draw(6, 0);

		blurEffect->UnBindShaderResource();
		blurEffect->UnBindConstantBuffer();

		blurEffect->UnSetShader();
	}

	void D3D11Renderer::DrawGodRay() {
		//--------------------Post processing----------------------//
		m_d3d11DeviceContext->OMSetBlendState(m_additiveAlphaAddBS, NULL, 0xFFFFFFFF);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
		m_d3d11DeviceContext->PSSetSamplers(0, 1, &m_borderSamplerState);
		//m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Black));
		//m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		GodRayEffect* godRayFX = EffectsManager::Instance()->m_godrayEffect;
		m_d3d11DeviceContext->IASetInputLayout(NULL);

		XMFLOAT3 lightDir = LightManager::Instance()->m_dirLights[0]->direction;
		XMVECTOR lightPos = -400 * XMLoadFloat3(&lightDir);
		XMVECTOR lightPosH = XMVector3Transform(lightPos, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
		XMFLOAT4 lightPosHf;
		XMStoreFloat4(&lightPosHf, lightPosH);
		lightPosHf.x /= lightPosHf.w;
		lightPosHf.x = 0.5f + lightPosHf.x / 2;
		lightPosHf.y /= lightPosHf.w;
		lightPosHf.y = 1 - (0.5f + lightPosHf.y / 2.0f);
		lightPosHf.z /= lightPosHf.w;

		godRayFX->SetShaderResources(m_normalBufferSRV, 0);
		godRayFX->m_perFrameConstantBuffer.gLightPosH = lightPosHf;
		godRayFX->SetShader();
		godRayFX->UpdateConstantBuffer();
		godRayFX->BindConstantBuffer();
		godRayFX->BindShaderResource();

		m_d3d11DeviceContext->Draw(6, 0);

		godRayFX->UnBindConstantBuffer();
		godRayFX->UnBindShaderResource();
		godRayFX->UnSetShader();

		m_d3d11DeviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		m_d3d11DeviceContext->OMSetDepthStencilState(0, 0);
	}

	void D3D11Renderer::DrawDebug() {
		// draw debug line
		m_d3d11DeviceContext->OMSetDepthStencilState(m_noZWriteDSState, 1); // turn off z write
		m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_deferredRenderDepthStencilView);

		UINT stride = sizeof(Vertex::DebugLine);
		UINT offset = 0;

		DebugLineEffect* debugFX = EffectsManager::Instance()->m_debugLineEffect;
		debugFX->SetShader();
		m_d3d11DeviceContext->IASetInputLayout((ID3D11InputLayout *)debugFX->GetInputLayout());
		m_d3d11DeviceContext->IASetIndexBuffer(m_gridCoordIndexBufferGPU, DXGI_FORMAT_R32_UINT, 0);
		m_d3d11DeviceContext->IASetVertexBuffers(0, 1, &m_gridCoordVertexBufferGPU, &stride, &offset);
		debugFX->GetPerObjConstantBuffer()->ViewProj = XMMatrixTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

		debugFX->UpdateConstantBuffer();
		debugFX->BindConstantBuffer();

		// draw grid first
		m_d3d11DeviceContext->DrawIndexed(44, 6, 0);
		// then draw axes
		m_d3d11DeviceContext->DrawIndexed(6, 0, 0);

		debugFX->UnBindConstantBuffer();
		debugFX->UnSetShader();
	}

	ID3D11ShaderResourceView* D3D11Renderer::doCSBlur(ID3D11ShaderResourceView* blurImgSRV, int uavSlotIdx) {
		// vblur
		VBlurEffect* vBlurEffect = EffectsManager::Instance()->m_vblurEffect;
		vBlurEffect->SetShader();
		vBlurEffect->UpdateConstantBuffer();
		vBlurEffect->BindConstantBuffer();
		vBlurEffect->SetShaderResources(blurImgSRV, 0);

		vBlurEffect->BindShaderResource(uavSlotIdx);

		UINT numGroupsX = (UINT)ceilf(m_clientWidth / 256.0f);
		m_d3d11DeviceContext->Dispatch(numGroupsX, m_clientHeight, 1);

		vBlurEffect->UnBindConstantBuffer();
		vBlurEffect->UnbindUnorderedAccessViews();
		vBlurEffect->UnBindShaderResource();
		vBlurEffect->UnSetShader();

		//hblur
		HBlurEffect* hBlurEffect = EffectsManager::Instance()->m_hblurEffect;
		hBlurEffect->SetShader();
		hBlurEffect->UpdateConstantBuffer();
		hBlurEffect->BindConstantBuffer();
		hBlurEffect->SetShaderResources(vBlurEffect->GetOutputShaderResource(uavSlotIdx), 0);
		hBlurEffect->BindShaderResource(uavSlotIdx);

		UINT numGroupsY = (UINT)ceilf(m_clientHeight / 256.0f);
		m_d3d11DeviceContext->Dispatch(m_clientWidth, numGroupsY, 1);

		hBlurEffect->UnBindConstantBuffer();
		hBlurEffect->UnbindUnorderedAccessViews();
		hBlurEffect->UnBindShaderResource();
		hBlurEffect->UnSetShader();

		return hBlurEffect->GetOutputShaderResource(uavSlotIdx);
	}

	virtual void* GetDevice() override {
		return m_d3d11Device;
	}

	virtual void* GetDeviceContext() override {
		return m_d3d11DeviceContext;
	}

	void UpdateTitle(const char* str) override {
		SetWindowTextA(m_hMainWnd, str);
	}

	void DrawShadowMap() override {
		LightManager::Instance()->m_shadowMap->GatherShadowDrawCall();

		//dc->RSSetViewports(1, &m_viewPort);
	

		m_d3d11DeviceContext->RSSetState(static_cast<ID3D11RasterizerState*>(Renderer::Instance()->GetDepthRS()));
		m_d3d11DeviceContext->IASetInputLayout((ID3D11InputLayout *)EffectsManager::Instance()->m_shadowMapEffect->GetInputLayout());
		m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// todo
		ID3D11DepthStencilView* dsv = (ID3D11DepthStencilView*)LightManager::Instance()->m_shadowMap->GetRenderTarget();

		uint32_t width, height;
		LightManager::Instance()->m_shadowMap->GetDimension(width, height);

		ID3D11RenderTargetView* renderTargets[1] = { 0 };
		m_d3d11DeviceContext->OMSetRenderTargets(1, renderTargets, dsv);
		m_d3d11DeviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//glViewport(0, 0, width, height);

		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.Height = static_cast<float>(height);
		vp.Width = static_cast<float>(width);
		vp.MaxDepth = 1;
		vp.MinDepth = 0;

		m_d3d11DeviceContext->RSSetViewports(1, &vp);

		for (auto &cmd : m_shadowMapDrawList)
		{
			cmd.effect->SetShader();

			//if (m_currentVao != (uint64_t)cmd.m_vertexArrayObject)
			//{
			//	m_currentVao = (uint64_t)cmd.m_vertexArrayObject;
			//	glBindVertexArray((uint64_t)cmd.m_vertexArrayObject);
			//}

			ID3D11Buffer* vertexBuffer = static_cast<ID3D11Buffer*>(cmd.vertexBuffer);
			ID3D11Buffer* indexBuffer = static_cast<ID3D11Buffer*>(cmd.indexBuffer);

			m_d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &cmd.vertexStride, &cmd.vertexOffset);
			m_d3d11DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			for (auto &cbuffer : cmd.cbuffers)
			{
				cbuffer.Bind();
			}

			m_d3d11DeviceContext->DrawIndexed(cmd.elementCount, 0, 0);
		}

		m_shadowMapDrawList.clear();
	}

	void AddDeferredDrawCmd(DrawCmd &cmd)
	{
		m_deferredDrawList.push_back(std::move(cmd));
	}

	void AddShadowDrawCmd(DrawCmd &cmd) override
	{
		m_shadowMapDrawList.push_back(std::move(cmd));
	}

	void* GetGbuffer() override
	{
		return nullptr;
	}

private:
	int m_clientWidth;
	int m_clientHeight;
	Skybox* m_SkyBox;

	HINSTANCE	m_hInst;
	HWND		m_hMainWnd;

	UINT		m_4xMsaaQuality;

	ID3D11Device* m_d3d11Device;
	ID3D11DeviceContext* m_d3d11DeviceContext;
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
	D3D_DRIVER_TYPE m_d3dDriverType;

	D3D11_VIEWPORT m_screenViewpot;
	D3D11_VIEWPORT m_screenSuperSampleViewpot;
	ID3D11RasterizerState* m_wireFrameRS;
	ID3D11SamplerState* m_samplerState;
	ID3D11SamplerState* m_heightMapSamplerState;
	ID3D11SamplerState* m_shadowSamplerState;
	ID3D11SamplerState* m_borderSamplerState;

	ID3D11DepthStencilState* m_noZWriteDSState;
	ID3D11BlendState* m_additiveAlphaAddBS;

#pragma region DEDERRED_RENDER
	ID3D11RenderTargetView* m_diffuseBufferRTV;
	ID3D11RenderTargetView* m_normalBufferRTV;
	ID3D11RenderTargetView* m_specularBufferRTV;
	ID3D11RenderTargetView* m_positionBufferRTV;
	ID3D11RenderTargetView* m_edgeBufferRTV;
	ID3D11RenderTargetView* m_SSAORTV;
	ID3D11RenderTargetView* m_SSAORTV2;
	ID3D11RenderTargetView* m_deferredShadingRTV;

	ID3D11ShaderResourceView* m_randVecTexSRV;

	ID3D11ShaderResourceView* m_diffuseBufferSRV;
	ID3D11ShaderResourceView* m_normalBufferSRV;
	ID3D11ShaderResourceView* m_specularBufferSRV;
	ID3D11ShaderResourceView* m_positionBufferSRV;
	ID3D11ShaderResourceView* m_edgeBufferSRV;
	ID3D11ShaderResourceView* m_SSAOSRV;
	ID3D11ShaderResourceView* m_SSAOSRV2;
	ID3D11ShaderResourceView* m_deferredShadingSRV;

	ID3D11DepthStencilView* m_deferredRenderDepthStencilView;
	ID3D11ShaderResourceView* m_deferredRenderShaderResourceView;

#pragma endregion

	bool m_enable4xMsaa;

	ID3D11Buffer* m_gridCoordIndexBufferGPU;
	ID3D11Buffer* m_gridCoordVertexBufferGPU;

	ID3D11RasterizerState* m_depthRS;

	std::vector<DrawCmd> m_deferredDrawList;
	std::vector<DrawCmd> m_shadowMapDrawList;
};

Renderer* Renderer::_instance = nullptr;

Renderer* Renderer::Create(HINSTANCE hInstance, HWND hMainWnd)
{
	D3D11Renderer* renderer = new D3D11Renderer(hInstance, hMainWnd);
	_instance = static_cast<Renderer*>(renderer);
	return _instance;
}

#endif