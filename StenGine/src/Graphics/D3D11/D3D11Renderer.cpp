#include "Graphics/D3D11/D3D11Renderer.h"

#pragma warning(disable: 4267 4244 4311 4302)

namespace StenGine
{

D3D11Renderer::D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd) :
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

	void D3D11Renderer::Release()  {
		_instance = nullptr;
		delete this;
	}

	bool D3D11Renderer::Init(int32_t width, int32_t height, CreateWindowCallback createWindow)  {

		if (!createWindow(width, height, m_hInst, m_hMainWnd))
		{
			assert(false);
			return false;
		}

		m_clientWidth = width;
		m_clientHeight = height;

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
		dLight->intensity = { 1.5f, 1.5f, 1.5f, 1 };
		dLight->direction = Vec3(-0.5, -2, 1).Normalized();
		dLight->castShadow = 1;

		LightManager::Instance()->m_dirLights.push_back(dLight);
		LightManager::Instance()->m_shadowMap = new ShadowMap(2048, 2048);

		m_SkyBox = std::unique_ptr<Skybox>(new Skybox(std::wstring(L"Model/sunsetcube1024.dds")));

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
		vbd.StructureByteStride = 0;
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

		m_GBuffer.AddRenderTarget(m_diffuseBufferRTV);
		m_GBuffer.AddRenderTarget(m_normalBufferRTV);
		m_GBuffer.AddRenderTarget(m_specularBufferRTV);

		m_GBuffer.AddClearColor(SGColors::LightSteelBlue);
		m_GBuffer.AddClearColor(SGColors::Black);
		m_GBuffer.AddClearColor(SGColors::Black);

		m_GBuffer.AssignDepthStencil(m_deferredRenderDepthStencilView);

		m_drawTopologyMap[PrimitiveTopology::POINTLIST] = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		m_drawTopologyMap[PrimitiveTopology::LINELIST] = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		m_drawTopologyMap[PrimitiveTopology::TRIANGLELIST] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_drawTopologyMap[PrimitiveTopology::CONTROL_POINT_3_PATCHLIST] = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		m_drawTopologyMap[PrimitiveTopology::CONTROL_POINT_4_PATCHLIST] = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;


		m_deferredShadingRT.AddRenderTarget(m_deferredShadingRTV);
		m_deferredShadingRT.AddRenderTarget(m_SSAORTV);
		m_deferredShadingRT.AddClearColor(SGColors::LightSteelBlue);
		m_deferredShadingRT.AddClearColor(SGColors::LightSteelBlue);
		m_deferredShadingRT.AssignDepthStencil(m_depthStencilView);

		m_defaultRTNoDepth.AddRenderTarget(m_renderTargetView);
		m_defaultRTNoDepth.AssignDepthStencil(nullptr);

		m_defaultRTWithDepth.AddRenderTarget(m_renderTargetView);
		m_defaultRTWithDepth.AddClearColor(SGColors::LightSteelBlue);
		m_defaultRTWithDepth.AssignDepthStencil(m_depthStencilView);

		m_debugRT.AddRenderTarget(m_renderTargetView);
		m_debugRT.AssignDepthStencil(m_deferredRenderDepthStencilView);

		/*****************************uav***************************/

		for (int i = 0; i < 4; i++) {

			D3D11_TEXTURE2D_DESC blurredTexDesc;
			blurredTexDesc.Width = Renderer::Instance()->GetScreenWidth();
			blurredTexDesc.Height = Renderer::Instance()->GetScreenHeight();
			blurredTexDesc.MipLevels = 1;
			blurredTexDesc.ArraySize = 1;
			blurredTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			blurredTexDesc.SampleDesc.Count = 1;
			blurredTexDesc.SampleDesc.Quality = 0;
			blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
			blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
			blurredTexDesc.CPUAccessFlags = 0;
			blurredTexDesc.MiscFlags = 0;

			ID3D11Texture2D* blurredTex = 0;
			HR(m_d3d11Device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = blurredTexDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			HR(m_d3d11Device->CreateShaderResourceView(blurredTex, &srvDesc, &m_outputShaderResources[i]));

			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.Format = blurredTexDesc.Format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;

			HR(m_d3d11Device->CreateUnorderedAccessView(blurredTex, &uavDesc, &m_unorderedAccessViews[i]));

			ReleaseCOM(blurredTex);
		}

		EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::RENDER, [this]() {Draw(); });

		return true;
	}

	void D3D11Renderer::Draw()  {

		m_GBuffer.ClearDepth(m_d3d11DeviceContext);

		ID3D11SamplerState* samplerState[] = { m_samplerState, m_shadowSamplerState, m_heightMapSamplerState };
		m_d3d11DeviceContext->PSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->VSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->DSSetSamplers(0, 3, samplerState);
		m_d3d11DeviceContext->HSSetSamplers(0, 3, samplerState);

		m_d3d11DeviceContext->RSSetState(0);
		m_d3d11DeviceContext->OMSetDepthStencilState(0, 0);

		DrawShadowMap();
		DrawGBuffer();
		DrawDeferredShading();
		m_SkyBox->Draw();
		DrawBlurSSAOAndCombine();
		
		//DrawGodRay();
		DrawDebug();

		// TEST
		ImGui::NewFrame();
		GameObjectManager::Instance()->DrawMenu();
		ImGui::Render();

		ExecuteCmdList();

		// clean up
		m_d3d11DeviceContext->ClearState();
		m_d3d11DeviceContext->RSSetState(0);
		m_d3d11DeviceContext->OMSetDepthStencilState(0, 0);
		m_d3d11DeviceContext->OMSetRenderTargets(0, NULL, NULL);
		m_d3d11DeviceContext->RSSetScissorRects(0, 0);
		HR(m_swapChain->Present(0, 0));
	}

	float D3D11Renderer::GetAspectRatio()  {
		return static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight);
	}

	int D3D11Renderer::GetScreenWidth()  {
		return m_clientWidth;
	}

	int D3D11Renderer::GetScreenHeight()  {
		return m_clientHeight;
	}

	Skybox* D3D11Renderer::GetSkyBox()  {
		return m_SkyBox.get();
	}

	void* D3D11Renderer::GetDepthRS()  {
		return m_depthRS;
	}

	void D3D11Renderer::ExecuteCmdList()
	{
		for (auto &cmd : m_drawList)
		{
			if (cmd.flags & CmdFlag::BIND_FB)
			{
				cmd.framebuffer->SetRenderTarget(m_d3d11DeviceContext);
			}

			if (cmd.flags & CmdFlag::SET_SS)
			{
				if (cmd.scissorState.scissorTestEnabled)
				{
					const D3D11_RECT r = { (LONG)cmd.scissorState.x, (LONG)cmd.scissorState.y, (LONG)cmd.scissorState.width, (LONG)cmd.scissorState.height };
					m_d3d11DeviceContext->RSSetScissorRects(1, &r);
				}
				else
				{
					m_d3d11DeviceContext->RSSetScissorRects(1, 0);
				}
			}

			if (cmd.flags & CmdFlag::CLEAR_COLOR)
			{
				cmd.framebuffer->ClearColor(m_d3d11DeviceContext);
			}

			if (cmd.flags & CmdFlag::CLEAR_DEPTH)
			{
				cmd.framebuffer->ClearDepth(m_d3d11DeviceContext);
			}

			if (cmd.flags & CmdFlag::SET_VP)
			{
				m_d3d11DeviceContext->RSSetViewports(1, &cmd.viewport);
			}

			if (cmd.flags & CmdFlag::SET_RSSTATE)
			{
				m_d3d11DeviceContext->RSSetState(cmd.rsState);
			}

			if (cmd.flags & CmdFlag::SET_BS)
			{
				const float blendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
				auto bsState = GetBlendState(cmd.blendState);
				m_d3d11DeviceContext->OMSetBlendState(bsState, blendFactor, 0xffffffff);
			}

			if (cmd.flags & CmdFlag::SET_DS)
			{
				auto dsState = GetDepthState(cmd.depthState);
				m_d3d11DeviceContext->OMSetDepthStencilState(dsState, 0);
			}

			if (cmd.flags & CmdFlag::SET_CS)
			{
				auto rsState = GetRasterizerState(cmd.rasterizerState);
				m_d3d11DeviceContext->RSSetState(rsState);
			}

			if (cmd.flags & CmdFlag::DRAW || cmd.flags & CmdFlag::COMPUTE)
			{
				cmd.effect->SetShader();

				for (auto &cbuffer : cmd.cbuffers)
				{
					cbuffer.Bind();
				}

				cmd.srvs.Bind();
				cmd.uavs.Bind();

				if (cmd.flags & CmdFlag::DRAW)
				{
					m_d3d11DeviceContext->IASetPrimitiveTopology(m_drawTopologyMap[cmd.type]);
					m_d3d11DeviceContext->IASetInputLayout((ID3D11InputLayout *)cmd.inputLayout);

					if (cmd.vertexBuffer)
					{
						ID3D11Buffer* vertexBuffer = static_cast<ID3D11Buffer*>(cmd.vertexBuffer);
						UINT offset = 0;
						m_d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &cmd.vertexStride, &offset);
					}

					if (cmd.drawType == DrawType::INDEXED)
					{
						if (cmd.indexBuffer)
						{
							ID3D11Buffer* indexBuffer = static_cast<ID3D11Buffer*>(cmd.indexBuffer);
							m_d3d11DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
						}

						m_d3d11DeviceContext->DrawIndexed(cmd.elementCount, (int64_t)cmd.offset, cmd.vertexOffset);
					}
					else if (cmd.drawType == DrawType::ARRAY)
					{
						m_d3d11DeviceContext->Draw(cmd.elementCount, (UINT)cmd.offset);
					}
				}
				else if (cmd.flags & CmdFlag::COMPUTE)
				{
					m_d3d11DeviceContext->Dispatch(cmd.threadGroupX, cmd.threadGroupY, cmd.threadGroupZ);
				}
				
				cmd.srvs.Unbind();
				cmd.uavs.Unbind();
			}
		}

		m_drawList.clear();
	}

	void D3D11Renderer::DrawShadowMap()  {
		LightManager::Instance()->m_shadowMap->UpdateShadowMatrix();

		// m_d3d11DeviceContext->RSSetState(m_depthRS);       // TODO !!!!!!!!!!!!!!!!!!!!!! 

		uint32_t width, height;
		LightManager::Instance()->m_shadowMap->GetDimension(width, height);

		DrawCmd shadowcmd;

		shadowcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_DEPTH | CmdFlag::SET_RSSTATE;
		shadowcmd.framebuffer = &LightManager::Instance()->m_shadowMap->GetRenderTarget();
		shadowcmd.viewport = { 0, 0, (float)width, (float)height, 0, 1 };
		shadowcmd.rsState = m_depthRS;

		m_drawList.push_back(std::move(shadowcmd));

		for (auto &gatherShadowDrawCall : m_shadowDrawHandler)
		{
			gatherShadowDrawCall();
		}
	}

	void D3D11Renderer::DrawGBuffer() {
// TODO
//		m_d3d11DeviceContext->RSSetState(0);
//


		DrawCmd drawcmd;

		drawcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::SET_RSSTATE;
		drawcmd.framebuffer = &m_GBuffer;

		drawcmd.viewport = m_screenViewpot;
		drawcmd.rsState = 0;

		m_drawList.push_back(std::move(drawcmd));

		for (auto &gatherDrawCall : m_drawHandler)
		{
			gatherDrawCall();
		}
	}

	void D3D11Renderer::DrawDeferredShading() {
		DrawCmd clearCmd;

		clearCmd.flags = CmdFlag::BIND_FB;

		//-------------- composite deferred render target views AND SSAO-------------------//
		DrawCmd cmd;
		DeferredShadingPassEffect* effect = EffectsManager::Instance()->m_deferredShadingPassEffect.get();

		ConstantBuffer cbuffer0(0, sizeof(DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
		DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER* perFrameData = (DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();


		DirectionalLight viewDirLight;
		memcpy(&viewDirLight, LightManager::Instance()->m_dirLights[0], sizeof(DirectionalLight));

		Mat4 ViewInvTranspose = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix().Inverse().Transpose();

		viewDirLight.direction = (ViewInvTranspose * Vec4(viewDirLight.direction.data[0], viewDirLight.direction.data[1], viewDirLight.direction.data[2], 0)).xyz();
		perFrameData->gDirLight = viewDirLight;

		Vec4 camPos = CameraManager::Instance()->GetActiveCamera()->GetPos();
		camPos = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * camPos;
		perFrameData->gEyePosV = camPos;
		perFrameData->gProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetProjMatrix());
		perFrameData->gProjInv = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetProjMatrix().Inverse());

		cmd.srvs.AddSRV(m_diffuseBufferSRV, 0);
		cmd.srvs.AddSRV(m_normalBufferSRV, 1);
		cmd.srvs.AddSRV(m_specularBufferSRV, 2);
		cmd.srvs.AddSRV(m_deferredRenderShaderResourceView, 3);
		cmd.srvs.AddSRV(m_randVecTexSRV, 4);

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = 0;
		cmd.vertexBuffer = 0; // don't bind if 0
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = &m_deferredShadingRT;
		cmd.offset = (void*)(0);
		cmd.effect = effect;
		cmd.elementCount = 6;
		cmd.cbuffers.push_back(std::move(cbuffer0));

		m_drawList.push_back(std::move(cmd));
	}

	void D3D11Renderer::DrawBlurSSAOAndCombine() {
		ID3D11ShaderResourceView* nullSRV[16] = { 0 };
		ID3D11SamplerState* samplerState[] = { m_samplerState, m_shadowSamplerState };
		// -------compute shader blur ----------//
		m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		DrawCmd clearRTcmd;

		clearRTcmd.flags = CmdFlag::BIND_FB;

		clearRTcmd.framebuffer = &m_defaultRTNoDepth;

		AddDeferredDrawCmd(std::move(clearRTcmd));

		doCSBlur(m_SSAOSRV, 0);
		doCSBlur(m_deferredShadingSRV, 2);

		ID3D11ShaderResourceView* blurredSSAOSRV = m_outputShaderResources[1];//doCSBlur(m_SSAOSRV, 0);
		ID3D11ShaderResourceView* blurredSRV = m_outputShaderResources[3];//doCSBlur(m_deferredShadingSRV, 1);

		// ------ Screen Quad -------//
		BlurEffect* blurEffect = EffectsManager::Instance()->m_blurEffect.get();

		DrawCmd cmd;

		// TODO
		//m_d3d11DeviceContext->PSSetSamplers(0, 1, samplerState);

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = 0;
		cmd.vertexBuffer = 0; // don't bind if 0
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = &m_defaultRTWithDepth;
		cmd.offset = (void*)(0);
		cmd.effect = blurEffect;
		cmd.elementCount = 6;

		cmd.srvs.AddSRV(m_deferredShadingSRV, 0);
		cmd.srvs.AddSRV(blurredSSAOSRV/*m_SSAOSRV*/, 1);
		cmd.srvs.AddSRV(blurredSRV, 2);

		AddDeferredDrawCmd(std::move(cmd));
	}

	void D3D11Renderer::DrawDebug() {
		UINT stride = sizeof(Vertex::DebugLine);
		UINT offset = 0;

		DebugLineEffect* debugLineFX = EffectsManager::Instance()->m_debugLineEffect.get();

		DrawCmd cmd;

		cmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_DS | CmdFlag::DRAW;

		cmd.depthState.depthWriteEnable = false;
		cmd.effect = debugLineFX;
		cmd.inputLayout = debugLineFX->GetInputLayout();
		cmd.indexBuffer = m_gridCoordIndexBufferGPU;
		cmd.vertexBuffer = m_gridCoordVertexBufferGPU;

		ConstantBuffer cbuffer0(0, sizeof(DebugLineEffect::PEROBJ_CONSTANT_BUFFER), debugLineFX->m_perObjectCB);
		DebugLineEffect::PEROBJ_CONSTANT_BUFFER* perObjectData = (DebugLineEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0.GetBuffer();

		perObjectData->ViewProj = CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix().Transpose();

		cmd.cbuffers.push_back(std::move(cbuffer0));
		cmd.type = PrimitiveTopology::LINELIST;
		cmd.drawType = DrawType::INDEXED;

		cmd.offset = (void*)6;
		cmd.elementCount = 44;
		cmd.vertexStride = stride;
		cmd.vertexOffset = offset;
		cmd.framebuffer = &m_debugRT;

		DrawCmd cmd2;

		cmd2.effect = debugLineFX;
		cmd2.flags = CmdFlag::DRAW;
		cmd2.offset = 0;
		cmd2.elementCount = 6;
		cmd2.type = PrimitiveTopology::LINELIST;
		cmd2.drawType = DrawType::INDEXED;
		cmd2.vertexStride = stride;
		cmd2.vertexOffset = offset;

		AddDeferredDrawCmd(cmd);
		AddDeferredDrawCmd(cmd2);
	}

	void D3D11Renderer::doCSBlur(ID3D11ShaderResourceView* blurImgSRV, int uavSlotIdx) {
		// vblur
		DrawCmd cmdV;

		VBlurEffect* vBlurEffect = EffectsManager::Instance()->m_vblurEffect.get();
		UINT numGroupsX = (UINT)ceilf(m_clientWidth / 256.0f);

		cmdV.effect = vBlurEffect;
		cmdV.flags = CmdFlag::COMPUTE;
		cmdV.threadGroupX = numGroupsX;
		cmdV.threadGroupY = m_clientHeight;
		cmdV.threadGroupZ = 1;
		cmdV.srvs.AddSRV(blurImgSRV, 0);
		cmdV.uavs.AddUAV(m_unorderedAccessViews[uavSlotIdx], 0);

		AddDeferredDrawCmd(std::move(cmdV));

		// hblur
		DrawCmd cmdH;

		HBlurEffect* hBlurEffect = EffectsManager::Instance()->m_hblurEffect.get();
		UINT numGroupsY = (UINT)ceilf(m_clientHeight / 256.0f);

		cmdH.effect = hBlurEffect;
		cmdH.flags = CmdFlag::COMPUTE;
		cmdH.threadGroupX = m_clientWidth;
		cmdH.threadGroupY = numGroupsY;
		cmdH.threadGroupZ = 1;
		cmdH.srvs.AddSRV(m_outputShaderResources[uavSlotIdx], 0);
		cmdH.uavs.AddUAV(m_unorderedAccessViews[uavSlotIdx + 1], 0);

		AddDeferredDrawCmd(std::move(cmdH));
	}

	ID3D11BlendState* D3D11Renderer::GetBlendState(BlendState& blendState)
	{
		auto entry = m_blendStateMap.find(blendState);
		if (entry == m_blendStateMap.end())
		{
			static const D3D11_BLEND convertBlend[] =
			{
				D3D11_BLEND_ZERO, //
				D3D11_BLEND_ZERO,
				D3D11_BLEND_ONE,
				D3D11_BLEND_SRC_COLOR,
				D3D11_BLEND_INV_SRC_COLOR,
				D3D11_BLEND_SRC_ALPHA,
				D3D11_BLEND_INV_SRC_ALPHA,
				D3D11_BLEND_DEST_ALPHA,
				D3D11_BLEND_INV_DEST_ALPHA,
				D3D11_BLEND_DEST_COLOR,
				D3D11_BLEND_INV_DEST_COLOR,
				D3D11_BLEND_SRC_ALPHA_SAT,
			};

			static const D3D11_BLEND_OP convertBlendFunc[] =
			{
				D3D11_BLEND_OP_ADD, //
				D3D11_BLEND_OP_ADD,
				D3D11_BLEND_OP_SUBTRACT,
				D3D11_BLEND_OP_REV_SUBTRACT,
				D3D11_BLEND_OP_MIN,
				D3D11_BLEND_OP_MAX,
			};

			D3D11_BLEND_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.AlphaToCoverageEnable = false;
			desc.RenderTarget[blendState.index].BlendEnable = blendState.blendEnable;
			desc.RenderTarget[blendState.index].SrcBlend = convertBlend[(uint32_t)blendState.srcBlend];
			desc.RenderTarget[blendState.index].DestBlend = convertBlend[(uint32_t)blendState.destBlend];
			desc.RenderTarget[blendState.index].BlendOp = convertBlendFunc[(uint32_t)blendState.blendOpColor];
			desc.RenderTarget[blendState.index].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[blendState.index].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[blendState.index].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[blendState.index].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			ID3D11BlendState* d3d11blendState;
			m_d3d11Device->CreateBlendState(&desc, &d3d11blendState);

			m_blendStateMap[blendState] = d3d11blendState;
		}

		return m_blendStateMap[blendState];
	}

	ID3D11DepthStencilState* D3D11Renderer::GetDepthState(DepthState& depthState)
	{
		auto entry = m_depthStateMap.find(depthState);
		if (entry == m_depthStateMap.end())
		{
			static const D3D11_COMPARISON_FUNC convertDepthFunc[] =
			{
				D3D11_COMPARISON_NEVER,
				D3D11_COMPARISON_NEVER,
				D3D11_COMPARISON_LESS,
				D3D11_COMPARISON_EQUAL,
				D3D11_COMPARISON_LESS_EQUAL,
				D3D11_COMPARISON_GREATER,
				D3D11_COMPARISON_NOT_EQUAL,
				D3D11_COMPARISON_GREATER_EQUAL,
				D3D11_COMPARISON_ALWAYS,
			};

			D3D11_DEPTH_STENCIL_DESC dsDesc;

			// Depth test parameters
			dsDesc.DepthEnable = depthState.depthCompEnable;
			dsDesc.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)depthState.depthWriteEnable;
			dsDesc.DepthFunc = convertDepthFunc[(uint32_t)depthState.depthFunc];

			// Stencil test parameters
			dsDesc.StencilEnable = false;
			//dsDesc.StencilReadMask = 0xFF;
			//dsDesc.StencilWriteMask = 0xFF;
			//
			//// Stencil operations if pixel is front-facing
			//dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			//dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			//dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			//dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			//
			//// Stencil operations if pixel is back-facing
			//dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			//dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			//dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			//dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			ID3D11DepthStencilState* d3d11depthState;

			m_d3d11Device->CreateDepthStencilState(&dsDesc, &d3d11depthState);

			m_depthStateMap[depthState] = d3d11depthState;
		}

		return m_depthStateMap[depthState];
	}

	ID3D11RasterizerState* D3D11Renderer::GetRasterizerState(RasterizerState & rasterizerState)
	{
		auto entry = m_rasterizerStateMap.find(rasterizerState);
		if (entry == m_rasterizerStateMap.end())
		{
			static const D3D11_CULL_MODE convertType[] =
			{
				D3D11_CULL_NONE,
				D3D11_CULL_FRONT,
				D3D11_CULL_BACK,
			};

			D3D11_RASTERIZER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_NONE;

			if (rasterizerState.cullFaceEnabled)
			{
				desc.CullMode = convertType[(uint32_t)rasterizerState.cullType];
			}

			desc.ScissorEnable = true;
			desc.DepthClipEnable = false;

			ID3D11RasterizerState* d3d11rasterizerState;

			m_d3d11Device->CreateRasterizerState(&desc, &d3d11rasterizerState);
			m_rasterizerStateMap[rasterizerState] = d3d11rasterizerState;
		}

		return m_rasterizerStateMap[rasterizerState];
	}

	void* D3D11Renderer::GetDevice()  {
		return m_d3d11Device;
	}

	void* D3D11Renderer::GetDeviceContext()  {
		return m_d3d11DeviceContext;
	}

	void D3D11Renderer::D3D11Renderer::UpdateTitle(const char* str)  {
		SetWindowTextA(m_hMainWnd, str);
	}

	void D3D11Renderer::AddDeferredDrawCmd(DrawCmd &cmd)
	{
		m_drawList.push_back(std::move(cmd));
	}

	RenderTarget &D3D11Renderer::GetGbuffer() 
	{
		return m_GBuffer;
	}

	void D3D11Renderer::AddDraw(DrawEventHandler handler)
	{
		m_drawHandler.push_back(handler);
	}

	void D3D11Renderer::AddShadowDraw(DrawEventHandler handler)
	{
		m_shadowDrawHandler.push_back(handler);
	}

}
