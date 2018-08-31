#include "stdafx.h"

#include <unordered_map>

#include <imgui.h>

#include "Engine/EventSystem.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Color.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/D3D11/D3D11Buffer.h"
#include "Graphics/D3D11/D3D11ConstantBuffer.h"
#include "Graphics/D3D11/D3D11RenderTarget.h"
#include "Graphics/D3D11/D3D11Texture.h"
#include "Graphics/D3D11/D3D11UAVBinding.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Math/MathHelper.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/Terrain/Terrain.h"
#include "Scene/LightManager.h"
#include "Scene/CameraManager.h"
#include "Scene/GameObjectManager.h"

#pragma warning(disable: 4267 4244 4311 4302)

namespace StenGine
{

class D3D11Renderer : public Renderer
{
public:

	D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd) :
		mHInst(hInstance),
		mHMainWnd(hMainWnd),
		mD3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
		mEnable4xMsaa(false),
		m4xMsaaQuality(0),

		mD3d11Device(nullptr),
		mD3d11DeviceContext(nullptr),
		mSwapChain(nullptr),
		mDepthStencilBuffer(nullptr),
		mRenderTargetView(nullptr),
		mDepthStencilView(nullptr)
	{
		ZeroMemory(&mScreenViewpot, sizeof(D3D11_VIEWPORT));
	}

	~D3D11Renderer() {

		ReleaseCOM(mRenderTargetView);
		ReleaseCOM(mDepthStencilView);
		ReleaseCOM(mSwapChain);
		ReleaseCOM(mDepthStencilBuffer);

		if (mD3d11DeviceContext)
			mD3d11DeviceContext->ClearState();

		ReleaseCOM(mD3d11DeviceContext);
		ReleaseCOM(mD3d11Device);
	}

	void Release() final  {
		_instance = nullptr;
		delete this;
	}

	bool Init(int32_t width, int32_t height, CreateWindowCallback createWindow) final {

		if (!createWindow(width, height, mHInst, mHMainWnd))
		{
			assert(false);
			return false;
		}

		mClientWidth = width;
		mClientHeight = height;

		SetWindowText(mHMainWnd, L"StenGine");
		ShowWindow(mHMainWnd, SW_SHOW);

		SetFocus(mHMainWnd);

		UINT createDeviceFlags = 0;
#if (defined(DEBUG) || defined(_DEBUG))  
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevel;
		HRESULT hr = D3D11CreateDevice(
			0,
			mD3dDriverType,
			0,
			createDeviceFlags,
			0,
			0,
			D3D11_SDK_VERSION,
			&mD3d11Device,
			&featureLevel,
			&mD3d11DeviceContext);

		if (FAILED(hr)) {
			MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
			return false;
		}

		if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
			MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
			return false;
		}

		HR(mD3d11Device->CheckMultisampleQualityLevels(
			DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
		assert(m4xMsaaQuality > 0);

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = mClientWidth;
		sd.BufferDesc.Height = mClientHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		if (mEnable4xMsaa) {
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m4xMsaaQuality - 1;
		}
		else {
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = mHMainWnd;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		IDXGIDevice* dxgiDevice = 0;
		HR(mD3d11Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

		IDXGIAdapter* dxgiAdapter = 0;
		HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

		IDXGIFactory* dxgiFactory = 0;
		HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

		HR(dxgiFactory->CreateSwapChain(mD3d11Device, &sd, &mSwapChain));

		ReleaseCOM(dxgiDevice);
		ReleaseCOM(dxgiAdapter);
		ReleaseCOM(dxgiFactory);

		ID3D11Texture2D* backBuffer;
		HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
		HR(mD3d11Device->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));
		ReleaseCOM(backBuffer);

		// --------

		D3D11_TEXTURE2D_DESC depthStencilDesc;

		depthStencilDesc.Width = mClientWidth;
		depthStencilDesc.Height = mClientHeight;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		if (mEnable4xMsaa) {
			depthStencilDesc.SampleDesc.Count = 4;
			depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
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

		HR(mD3d11Device->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
		HR(mD3d11Device->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));


		mScreenViewpot.TopLeftX = 0;
		mScreenViewpot.TopLeftY = 0;
		mScreenViewpot.Width = static_cast<float>(mClientWidth);
		mScreenViewpot.Height = static_cast<float>(mClientHeight);
		mScreenViewpot.MinDepth = 0.0f;
		mScreenViewpot.MaxDepth = 1.0f;

		mScreenSuperSampleViewpot.TopLeftX = 0;
		mScreenSuperSampleViewpot.TopLeftY = 0;
		mScreenSuperSampleViewpot.Width = static_cast<float>(mClientWidth);
		mScreenSuperSampleViewpot.Height = static_cast<float>(mClientHeight);
		mScreenSuperSampleViewpot.MinDepth = 0.0f;
		mScreenSuperSampleViewpot.MaxDepth = 1.0f;

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
			hr = mD3d11Device->CreateSamplerState(&samplerDesc, &mSamplerState);
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
			hr = mD3d11Device->CreateSamplerState(&samplerDesc, &mHeightMapSamplerState);
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
			hr = mD3d11Device->CreateSamplerState(&shadowSamplerDesc, &mShadowSamplerState);
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
			hr = mD3d11Device->CreateSamplerState(&samplerDesc, &mBorderSamplerState);
			assert(SUCCEEDED(hr));
		}

		//-----------------setup MRT---------------------
		D3D11_TEXTURE2D_DESC gNormalBufferDesc;
		gNormalBufferDesc.Width = mClientWidth;
		gNormalBufferDesc.Height = mClientHeight;
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
		gBufferDesc.Width = mClientWidth;
		gBufferDesc.Height = mClientHeight;
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
		ID3D11Texture2D* gBufferMTex = 0;
		ID3D11Texture2D* gSSAOTex = 0;
		ID3D11Texture2D* gSSAOTex2 = 0;
		ID3D11Texture2D* gDeferredShadeTex = 0;
		//ID3D11Texture2D* gBufferETex = 0;
		HR(mD3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferDTex));
		HR(mD3d11Device->CreateTexture2D(&gNormalBufferDesc, 0, &gBufferNTex));
		HR(mD3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferSTex));
		HR(mD3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferMTex));
		HR(mD3d11Device->CreateTexture2D(&gBufferDesc, 0, &gSSAOTex));
		HR(mD3d11Device->CreateTexture2D(&gBufferDesc, 0, &gSSAOTex2));
		HR(mD3d11Device->CreateTexture2D(&gBufferDesc, 0, &gDeferredShadeTex));
		//HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferETex));

		D3D11_RENDER_TARGET_VIEW_DESC rtvNormalDesc;
		rtvNormalDesc.Format = gNormalBufferDesc.Format;
		rtvNormalDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvNormalDesc.Texture2D.MipSlice = 0;


		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = gBufferDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		HR(mD3d11Device->CreateRenderTargetView(gBufferDTex, &rtvDesc, &mDiffuseBufferRTV));
		HR(mD3d11Device->CreateRenderTargetView(gBufferNTex, &rtvNormalDesc, &mNormalBufferRTV));
		HR(mD3d11Device->CreateRenderTargetView(gBufferSTex, &rtvDesc, &mSpecularBufferRTV));
		HR(mD3d11Device->CreateRenderTargetView(gBufferMTex, &rtvDesc, &mMotionVecBufferRTV));
		HR(mD3d11Device->CreateRenderTargetView(gSSAOTex, &rtvDesc, &mSSAORTV));
		HR(mD3d11Device->CreateRenderTargetView(gSSAOTex2, &rtvDesc, &mSSAORTV2));
		HR(mD3d11Device->CreateRenderTargetView(gDeferredShadeTex, &rtvDesc, &mDeferredShadingRTV));
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

		HR(mD3d11Device->CreateShaderResourceView(gBufferDTex, &srvDesc, &mDiffuseBufferSRV));
		HR(mD3d11Device->CreateShaderResourceView(gBufferNTex, &srvNormalDesc, &mNormalBufferSRV));
		HR(mD3d11Device->CreateShaderResourceView(gBufferSTex, &srvDesc, &mSpecularBufferSRV));
		HR(mD3d11Device->CreateShaderResourceView(gBufferMTex, &srvDesc, &mMotionVecBufferSRV));
		HR(mD3d11Device->CreateShaderResourceView(gSSAOTex, &srvDesc, &mSSAOSRV));
		HR(mD3d11Device->CreateShaderResourceView(gSSAOTex2, &srvDesc, &mSSAOSRV2));
		HR(mD3d11Device->CreateShaderResourceView(gDeferredShadeTex, &srvDesc, &mDeferredShadingSRV));
		//HR(m_d3d11Device->CreateShaderResourceView(gBufferETex, &srvDesc, &m_edgeBufferSRV));

		ReleaseCOM(gBufferDTex);
		ReleaseCOM(gBufferNTex);
		ReleaseCOM(gBufferSTex);
		ReleaseCOM(gBufferMTex);
		ReleaseCOM(gSSAOTex);
		ReleaseCOM(gDeferredShadeTex);

		D3D11_TEXTURE2D_DESC depthTexDesc;
		depthTexDesc.Width = mClientWidth;
		depthTexDesc.Height = mClientHeight;
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
		HR(mD3d11Device->CreateTexture2D(&depthTexDesc, 0, &depthTex));

		// Create the depth stencil view for the entire cube
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Flags = 0;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		HR(mD3d11Device->CreateDepthStencilView(depthTex, &dsvDesc, &mDeferredRenderDepthStencilView));

		D3D11_SHADER_RESOURCE_VIEW_DESC dsrvDesc;
		dsrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		dsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dsrvDesc.Texture2D.MipLevels = depthTexDesc.MipLevels;
		dsrvDesc.Texture2D.MostDetailedMip = 0;
		HR(mD3d11Device->CreateShaderResourceView(depthTex, &dsrvDesc, &mDeferredRenderShaderResourceView));

		ReleaseCOM(depthTex);

		D3D11_RASTERIZER_DESC wireframeDesc;
		ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
		wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
		wireframeDesc.CullMode = D3D11_CULL_BACK;
		wireframeDesc.FrontCounterClockwise = false;
		wireframeDesc.DepthClipEnable = true;

		HR(mD3d11Device->CreateRasterizerState(&wireframeDesc, &mWireFrameRS));

		D3D11_RASTERIZER_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(D3D11_RASTERIZER_DESC));
		depthDesc.FillMode = D3D11_FILL_SOLID;
		depthDesc.CullMode = D3D11_CULL_BACK;
		depthDesc.DepthBias = 100000;
		depthDesc.FrontCounterClockwise = false;
		depthDesc.DepthClipEnable = true;
		depthDesc.DepthBiasClamp = 0.0f;
		depthDesc.SlopeScaledDepthBias = 1.0f;

		HR(mD3d11Device->CreateRasterizerState(&depthDesc, &mDepthRS));


		DirectionalLight* dLight = new DirectionalLight();
		dLight->intensity = { 1.5f, 1.5f, 1.5f, 1 };
		dLight->direction = Vec3(-0.5, -2, 1).Normalized();
		dLight->castShadow = 1;

		LightManager::Instance()->m_dirLights.push_back(dLight);
		LightManager::Instance()->m_shadowMap = new ShadowMap(2048, 2048);

		mSkyBox = std::unique_ptr<Skybox>(new Skybox(std::wstring(L"Model/sunsetcube1024.dds")));

		mD3d11DeviceContext->RSSetState(mWireFrameRS);

		DirectX::CreateDDSTextureFromFile(mD3d11Device,
			L"./Model/RandNorm.dds", nullptr, &mRandVecTexSRV);

		// init grid and coord debug draw
		std::vector<Vec3Packed> coordVertexBuffer = {
			Vec3Packed({ 0, 0, 0 }),
			Vec3Packed({ 5, 0, 0 }),
			Vec3Packed({ 0, 0, 0 }),
			Vec3Packed({ 0, 5, 0 }),
			Vec3Packed({ 0, 0, 0 }),
			Vec3Packed({ 0, 0, 5 }),
		};

		std::vector<UINT> coordIndexBuffer = { 0, 1, 2, 3, 4, 5 };


		int initIdx = 6;
		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.emplace_back(Vec3(-5.f, 0.f, -5.f + i));
			coordVertexBuffer.emplace_back(Vec3(5.f, 0.f, -5.f + i));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.emplace_back(Vec3(-5.f + i, 0.f, -5.f));
			coordVertexBuffer.emplace_back(Vec3(-5.f + i, 0.f, 5.f));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vec3Packed) * coordVertexBuffer.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &coordVertexBuffer[0];
		HR(mD3d11Device->CreateBuffer(&vbd, &vinitData, &mGridCoordVertexBufferGPU));

		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * coordIndexBuffer.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &coordIndexBuffer[0];
		HR(mD3d11Device->CreateBuffer(&ibd, &iinitData, &mGridCoordIndexBufferGPU));

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

			mD3d11Device->CreateDepthStencilState(&dsDesc, &mNoZWriteDSState);
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

			HRESULT hr = mD3d11Device->CreateBlendState(&bsDesc, &mAdditiveAlphaAddBS);
			assert(SUCCEEDED(hr));
		}

		mGBuffer = CreateRenderTarget();
		mGBuffer->AddRenderTarget(mDiffuseBufferRTV);
		mGBuffer->AddRenderTarget(mNormalBufferRTV);
		mGBuffer->AddRenderTarget(mSpecularBufferRTV);
		mGBuffer->AddRenderTarget(mMotionVecBufferRTV);

		mGBuffer->AddClearColor(SGColors::LightSteelBlue);
		mGBuffer->AddClearColor(SGColors::Black);
		mGBuffer->AddClearColor(SGColors::Black);
		mGBuffer->AddClearColor(SGColors::Black);

		mGBuffer->AssignDepthStencil(mDeferredRenderDepthStencilView);

		mDrawTopologyMap[PrimitiveTopology::POINTLIST] = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		mDrawTopologyMap[PrimitiveTopology::LINELIST] = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		mDrawTopologyMap[PrimitiveTopology::TRIANGLELIST] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		mDrawTopologyMap[PrimitiveTopology::CONTROL_POINT_3_PATCHLIST] = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		mDrawTopologyMap[PrimitiveTopology::CONTROL_POINT_4_PATCHLIST] = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

		mDeferredShadingRT = CreateRenderTarget();
		mDeferredShadingRT->AddRenderTarget(mDeferredShadingRTV);
		mDeferredShadingRT->AddRenderTarget(mSSAORTV);
		mDeferredShadingRT->AddClearColor(SGColors::LightSteelBlue);
		mDeferredShadingRT->AddClearColor(SGColors::LightSteelBlue);
		mDeferredShadingRT->AssignDepthStencil(mDepthStencilView);

		mDefaultRTNoDepth = CreateRenderTarget();
		mDefaultRTNoDepth->AddRenderTarget(mRenderTargetView);
		mDefaultRTNoDepth->AssignDepthStencil(nullptr);

		mDefaultRTWithDepth = CreateRenderTarget();
		mDefaultRTWithDepth->AddRenderTarget(mRenderTargetView);
		mDefaultRTWithDepth->AddClearColor(SGColors::LightSteelBlue);
		mDefaultRTWithDepth->AssignDepthStencil(mDepthStencilView);

		mDebugRT = CreateRenderTarget();
		mDebugRT->AddRenderTarget(mRenderTargetView);
		mDebugRT->AssignDepthStencil(mDeferredRenderDepthStencilView);

		/*****************************uav***************************/

		for (int i = 0; i < 4; i++) {

			D3D11_TEXTURE2D_DESC blurredTexDesc;
			blurredTexDesc.Width = Renderer::Instance()->GetScreenWidth();
			blurredTexDesc.Height = Renderer::Instance()->GetScreenHeight();
			blurredTexDesc.MipLevels = 1;
			blurredTexDesc.ArraySize = 1;
			blurredTexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			blurredTexDesc.SampleDesc.Count = 1;
			blurredTexDesc.SampleDesc.Quality = 0;
			blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
			blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
			blurredTexDesc.CPUAccessFlags = 0;
			blurredTexDesc.MiscFlags = 0;

			ID3D11Texture2D* blurredTex = 0;
			HR(mD3d11Device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = blurredTexDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			HR(mD3d11Device->CreateShaderResourceView(blurredTex, &srvDesc, &mOutputShaderResources[i]));

			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.Format = blurredTexDesc.Format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;

			HR(mD3d11Device->CreateUnorderedAccessView(blurredTex, &uavDesc, &mUnorderedAccessViews[i]));

			ReleaseCOM(blurredTex);
		}

		EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::RENDER, [this]() {Draw(); });

		return true;
	}

	void Draw() final {

		mGBuffer->ClearDepth(mD3d11DeviceContext);

		ID3D11SamplerState* samplerState[] = { mSamplerState, mShadowSamplerState, mHeightMapSamplerState };
		mD3d11DeviceContext->PSSetSamplers(0, 3, samplerState);
		mD3d11DeviceContext->VSSetSamplers(0, 3, samplerState);
		mD3d11DeviceContext->DSSetSamplers(0, 3, samplerState);
		mD3d11DeviceContext->HSSetSamplers(0, 3, samplerState);

		mD3d11DeviceContext->RSSetState(0);
		mD3d11DeviceContext->OMSetDepthStencilState(0, 0);

		DrawShadowMap();
		DrawGBuffer();
		DrawDeferredShading();
		mSkyBox->Draw();
		DrawBlurSSAOAndCombine();
		
		//DrawGodRay();
		DrawDebug();

		// TEST
		ImGui::NewFrame();
		GameObjectManager::Instance()->DrawMenu();
		ImGui::Render();

		ExecuteCmdList();

		// clean up
		mD3d11DeviceContext->ClearState();
		mD3d11DeviceContext->RSSetState(0);
		mD3d11DeviceContext->OMSetDepthStencilState(0, 0);
		mD3d11DeviceContext->OMSetRenderTargets(0, NULL, NULL);
		mD3d11DeviceContext->RSSetScissorRects(0, 0);
		HR(mSwapChain->Present(0, 0));
	}

	float GetAspectRatio() final {
		return static_cast<float>(mClientWidth) / static_cast<float>(mClientHeight);
	}

	int GetScreenWidth() final {
		return mClientWidth;
	}

	int GetScreenHeight() final {
		return mClientHeight;
	}

	Skybox* GetSkyBox() final {
		return mSkyBox.get();
	}

	void* GetDepthRS() final {
		return mDepthRS;
	}

	void DrawShadowMap() final {
		LightManager::Instance()->m_shadowMap->UpdateShadowMatrix();

		// m_d3d11DeviceContext->RSSetState(m_depthRS);       // TODO !!!!!!!!!!!!!!!!!!!!!! 

		uint32_t width, height;
		LightManager::Instance()->m_shadowMap->GetDimension(width, height);

		DrawCmd shadowcmd;

		shadowcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_DEPTH | CmdFlag::SET_RSSTATE;
		shadowcmd.framebuffer = LightManager::Instance()->m_shadowMap->GetRenderTarget();
		shadowcmd.viewport = { 0, 0, (float)width, (float)height, 0, 1 };
		shadowcmd.rsState = mDepthRS;

		mDrawList.push_back(std::move(shadowcmd));

		for (auto &gatherShadowDrawCall : mShadowDrawHandler)
		{
			gatherShadowDrawCall();
		}
	}

	void DrawGBuffer() final {
// TODO
//		m_d3d11DeviceContext->RSSetState(0);
//


		DrawCmd drawcmd;

		drawcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::SET_RSSTATE;
		drawcmd.framebuffer = mGBuffer;

		drawcmd.viewport = mScreenViewpot;
		drawcmd.rsState = 0;

		mDrawList.push_back(std::move(drawcmd));

		for (auto &gatherDrawCall : mDrawHandler)
		{
			gatherDrawCall();
		}
	}

	void DrawDeferredShading() final {
		DrawCmd clearCmd;

		clearCmd.flags = CmdFlag::BIND_FB;

		//-------------- composite deferred render target views AND SSAO-------------------//
		DrawCmd cmd;
		DeferredShadingPassEffect* effect = EffectsManager::Instance()->m_deferredShadingPassEffect.get();

		ConstantBuffer cbuffer0 = CreateConstantBuffer(0, sizeof(DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
		DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER* perFrameData = (DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();


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

		cmd.srvs.AddSRV(mDiffuseBufferSRV, 0);
		cmd.srvs.AddSRV(mNormalBufferSRV, 1);
		cmd.srvs.AddSRV(mSpecularBufferSRV, 2);
		cmd.srvs.AddSRV(mDeferredRenderShaderResourceView, 3);
		cmd.srvs.AddSRV(mRandVecTexSRV, 4);

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = 0;
		cmd.vertexBuffer.push_back(0); // don't bind if 0
		cmd.vertexOffset.push_back(0);
		cmd.vertexStride.push_back(0);
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = mDeferredShadingRT;
		cmd.offset = (void*)(0);
		cmd.effect = effect;
		cmd.elementCount = 6;
		cmd.cbuffers.push_back(std::move(cbuffer0));

		mDrawList.push_back(std::move(cmd));
	}

	void DrawBlurSSAOAndCombine() final {
		ID3D11ShaderResourceView* nullSRV[16] = { 0 };
		ID3D11SamplerState* samplerState[] = { mSamplerState, mShadowSamplerState };
		// -------compute shader blur ----------//
		mD3d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		DrawCmd clearRTcmd;

		clearRTcmd.flags = CmdFlag::BIND_FB;

		clearRTcmd.framebuffer = mDefaultRTNoDepth;

		AddDeferredDrawCmd(std::move(clearRTcmd));

		doCSBlur(mSSAOSRV, 0);
		doCSBlur(mDeferredShadingSRV, 2);

		ID3D11ShaderResourceView* blurredSSAOSRV = mOutputShaderResources[1];//doCSBlur(m_SSAOSRV, 0);
		ID3D11ShaderResourceView* blurredSRV = mOutputShaderResources[3];//doCSBlur(m_deferredShadingSRV, 1);

		// ------ Screen Quad -------//
		BlurEffect* blurEffect = EffectsManager::Instance()->m_blurEffect.get();

		DrawCmd cmd;

		// TODO
		//m_d3d11DeviceContext->PSSetSamplers(0, 1, samplerState);

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = 0;
		cmd.vertexBuffer.push_back(0); // don't bind if 0
		cmd.vertexOffset.push_back(0);
		cmd.vertexStride.push_back(0);
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = mDefaultRTWithDepth;
		cmd.offset = (void*)(0);
		cmd.effect = blurEffect;
		cmd.elementCount = 6;

		cmd.srvs.AddSRV(mDeferredShadingSRV, 0);
		cmd.srvs.AddSRV(blurredSSAOSRV/*m_SSAOSRV*/, 1);
		cmd.srvs.AddSRV(blurredSRV, 2);

		AddDeferredDrawCmd(std::move(cmd));
	}

	void DrawDebug() final {
		UINT stride = sizeof(Vertex::DebugLine);
		UINT offset = 0;

		DebugLineEffect* debugLineFX = EffectsManager::Instance()->m_debugLineEffect.get();

		// Draw grid
		{
			DrawCmd cmd;

			cmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_DS | CmdFlag::DRAW;

			cmd.depthState.depthWriteEnable = false;
			cmd.effect = debugLineFX;
			cmd.inputLayout = debugLineFX->GetInputLayout();
			cmd.indexBuffer = mGridCoordIndexBufferGPU;
			cmd.vertexBuffer.push_back(mGridCoordVertexBufferGPU);

			ConstantBuffer cbuffer0 = CreateConstantBuffer(0, sizeof(DebugLineEffect::PEROBJ_CONSTANT_BUFFER), debugLineFX->m_perObjectCB);
			DebugLineEffect::PEROBJ_CONSTANT_BUFFER* perObjectData = (DebugLineEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0->GetBuffer();

			perObjectData->ViewProj = CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix().Transpose();

			cmd.cbuffers.push_back(std::move(cbuffer0));
			cmd.type = PrimitiveTopology::LINELIST;
			cmd.drawType = DrawType::INDEXED;

			cmd.offset = (void*)6;
			cmd.elementCount = 44;
			cmd.vertexBuffer.push_back(0);
			cmd.vertexStride.push_back(stride);
			cmd.vertexOffset.push_back(offset);
			cmd.framebuffer = mDebugRT;
			AddDeferredDrawCmd(cmd);
		}

		// draw axes
		{
			DrawCmd cmd2;

			cmd2.flags = CmdFlag::DRAW;

			cmd2.effect = debugLineFX;
			cmd2.inputLayout = debugLineFX->GetInputLayout();
			cmd2.indexBuffer = mGridCoordIndexBufferGPU;
			cmd2.vertexBuffer.push_back(mGridCoordVertexBufferGPU);

			ConstantBuffer cbuffer0 = CreateConstantBuffer(0, sizeof(DebugLineEffect::PEROBJ_CONSTANT_BUFFER), debugLineFX->m_perObjectCB);
			DebugLineEffect::PEROBJ_CONSTANT_BUFFER* perObjectData = (DebugLineEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0->GetBuffer();

			perObjectData->ViewProj = CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix().Transpose();

			cmd2.cbuffers.push_back(std::move(cbuffer0));
			cmd2.type = PrimitiveTopology::LINELIST;
			cmd2.drawType = DrawType::INDEXED;

			cmd2.offset = 0;
			cmd2.elementCount = 6;
			cmd2.vertexBuffer.push_back(0);
			cmd2.vertexStride.push_back(stride);
			cmd2.vertexOffset.push_back(offset);
			AddDeferredDrawCmd(cmd2);
		}
	}

	void* GetDevice() final {
		return mD3d11Device;
	}

	void* GetDeviceContext() final {
		return mD3d11DeviceContext;
	}

	void UpdateTitle(const char* str) final {
		SetWindowTextA(mHMainWnd, str);
	}

	void AddDeferredDrawCmd(DrawCmd &cmd) final
	{
		mDrawList.push_back(std::move(cmd));
	}

	RenderTarget &GetGbuffer() final
	{
		return mGBuffer;
	}

	void AddDraw(DrawEventHandler handler) final
	{
		mDrawHandler.push_back(handler);
	}

	void AddShadowDraw(DrawEventHandler handler) final
	{
		mShadowDrawHandler.push_back(handler);
	}

	void EndFrame() final { /*TODO*/ }

	void AcquireContext() final { /*TODO*/ }

	void ReleaseContext() final { /*TODO*/ }

	ConstantBuffer CreateConstantBuffer(uint32_t index, uint32_t size, GPUBuffer buffer) final
	{
		return std::make_unique<D3D11ConstantBuffer>(index, size, buffer);
	}

	GPUBuffer CreateGPUBuffer(size_t size, BufferUsage usage, void* data /*= nullptr*/, BufferType type /*= BufferType::GENERAL*/) final
	{
		return std::make_shared<D3D11Buffer>(size, usage, data, type);
	}

	RenderTarget CreateRenderTarget() final
	{
		return std::make_shared<D3D11RenderTarget>();
	}

	UAVBinding CreateUAVBinding() final
	{
		return std::make_unique<D3D11UAVBinding>();
	}

	Texture CreateTexture(uint32_t width, uint32_t height, void* srv) final
	{
		return std::make_shared<D3D11Texture>(width, height, srv);
	}

private:

	void ExecuteCmdList()
	{
		for (auto &cmd : mDrawList)
		{
			if (cmd.flags & CmdFlag::BIND_FB)
			{
				cmd.framebuffer->SetRenderTarget(mD3d11DeviceContext);
			}

			if (cmd.flags & CmdFlag::SET_SS)
			{
				if (cmd.scissorState.scissorTestEnabled)
				{
					const D3D11_RECT r = { (LONG)cmd.scissorState.x, (LONG)cmd.scissorState.y, (LONG)cmd.scissorState.width, (LONG)cmd.scissorState.height };
					mD3d11DeviceContext->RSSetScissorRects(1, &r);
				}
				else
				{
					mD3d11DeviceContext->RSSetScissorRects(1, 0);
				}
			}

			if (cmd.flags & CmdFlag::CLEAR_COLOR)
			{
				cmd.framebuffer->ClearColor(mD3d11DeviceContext);
			}

			if (cmd.flags & CmdFlag::CLEAR_DEPTH)
			{
				cmd.framebuffer->ClearDepth(mD3d11DeviceContext);
			}

			if (cmd.flags & CmdFlag::SET_VP)
			{
				mD3d11DeviceContext->RSSetViewports(1, &cmd.viewport);
			}

			if (cmd.flags & CmdFlag::SET_RSSTATE)
			{
				mD3d11DeviceContext->RSSetState(cmd.rsState);
			}

			if (cmd.flags & CmdFlag::SET_BS)
			{
				const float blendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
				auto bsState = GetBlendState(cmd.blendState);
				mD3d11DeviceContext->OMSetBlendState(bsState, blendFactor, 0xffffffff);
			}

			if (cmd.flags & CmdFlag::SET_DS)
			{
				auto dsState = GetDepthState(cmd.depthState);
				mD3d11DeviceContext->OMSetDepthStencilState(dsState, 0);
			}

			if (cmd.flags & CmdFlag::SET_CS)
			{
				auto rsState = GetRasterizerState(cmd.rasterizerState);
				mD3d11DeviceContext->RSSetState(rsState);
			}

			if (cmd.flags & CmdFlag::DRAW || cmd.flags & CmdFlag::COMPUTE)
			{
				cmd.effect->SetShader();

				for (auto &cbuffer : cmd.cbuffers)
				{
					cbuffer->Bind();
				}

				cmd.srvs.Bind();

				if (cmd.flags & CmdFlag::COMPUTE)
				{
					cmd.uavs->Bind();
				}

				if (cmd.flags & CmdFlag::DRAW)
				{
					mD3d11DeviceContext->IASetPrimitiveTopology(mDrawTopologyMap[cmd.type]);
					mD3d11DeviceContext->IASetInputLayout((ID3D11InputLayout *)cmd.inputLayout);

					if (cmd.vertexBuffer.size())
					{
						// ID3D11Buffer* vertexBuffer = reinterpret_cast<ID3D11Buffer*>(cmd.vertexBuffer[0]);
						std::vector<ID3D11Buffer*> vbs;
						std::vector<UINT> offsets;
						for (auto &vb : cmd.vertexBuffer)
						{
							vbs.push_back((ID3D11Buffer*)vb);
							offsets.push_back(0);
						}
						mD3d11DeviceContext->IASetVertexBuffers(0, cmd.vertexBuffer.size(), &vbs.front(), cmd.vertexStride.data(), offsets.data());
					}

					if (cmd.drawType == DrawType::INDEXED)
					{
						if (cmd.indexBuffer)
						{
							ID3D11Buffer* indexBuffer = static_cast<ID3D11Buffer*>(cmd.indexBuffer);
							mD3d11DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
						}

						if (cmd.instanceCount)
						{
							mD3d11DeviceContext->DrawIndexedInstanced(cmd.elementCount, cmd.instanceCount, (int64_t)cmd.offset, cmd.vertexOffset[0], 0);
						}
						else
						{
							mD3d11DeviceContext->DrawIndexed(cmd.elementCount, (int64_t)cmd.offset, cmd.vertexOffset[0]);
						}
					}
					else if (cmd.drawType == DrawType::ARRAY)
					{
						if (cmd.instanceCount)
						{
							assert(0); // TODO
						}
						else
						{
							mD3d11DeviceContext->Draw(cmd.elementCount, (UINT)cmd.offset);
						}
					}
				}
				else if (cmd.flags & CmdFlag::COMPUTE)
				{
					mD3d11DeviceContext->Dispatch(cmd.threadGroupX, cmd.threadGroupY, cmd.threadGroupZ);
				}

				cmd.srvs.Unbind();

				if (cmd.flags & CmdFlag::COMPUTE)
				{
					cmd.uavs->Unbind();
				}
			}
		}

		mDrawList.clear();
	}

	void doCSBlur(ID3D11ShaderResourceView* blurImgSRV, int uavSlotIdx) {
		// vblur
		DrawCmd cmdV;

		VBlurEffect* vBlurEffect = EffectsManager::Instance()->m_vblurEffect.get();
		UINT numGroupsX = (UINT)ceilf(mClientWidth / 256.0f);

		cmdV.effect = vBlurEffect;
		cmdV.flags = CmdFlag::COMPUTE;
		cmdV.threadGroupX = numGroupsX;
		cmdV.threadGroupY = mClientHeight;
		cmdV.threadGroupZ = 1;
		cmdV.srvs.AddSRV(blurImgSRV, 0);
		cmdV.uavs = CreateUAVBinding();
		cmdV.uavs->AddUAV(mUnorderedAccessViews[uavSlotIdx], 0);

		AddDeferredDrawCmd(std::move(cmdV));

		// hblur
		DrawCmd cmdH;

		HBlurEffect* hBlurEffect = EffectsManager::Instance()->m_hblurEffect.get();
		UINT numGroupsY = (UINT)ceilf(mClientHeight / 256.0f);

		cmdH.effect = hBlurEffect;
		cmdH.flags = CmdFlag::COMPUTE;
		cmdH.threadGroupX = mClientWidth;
		cmdH.threadGroupY = numGroupsY;
		cmdH.threadGroupZ = 1;
		cmdH.srvs.AddSRV(mOutputShaderResources[uavSlotIdx], 0);
		cmdH.uavs = CreateUAVBinding();
		cmdH.uavs->AddUAV(mUnorderedAccessViews[uavSlotIdx + 1], 0);

		AddDeferredDrawCmd(std::move(cmdH));
	}

	ID3D11BlendState* GetBlendState(BlendState& blendState)
	{
		auto entry = mBlendStateMap.find(blendState);
		if (entry == mBlendStateMap.end())
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
			mD3d11Device->CreateBlendState(&desc, &d3d11blendState);

			mBlendStateMap[blendState] = d3d11blendState;
		}

		return mBlendStateMap[blendState];
	}

	ID3D11DepthStencilState* GetDepthState(DepthState& depthState)
	{
		auto entry = mDepthStateMap.find(depthState);
		if (entry == mDepthStateMap.end())
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

			mD3d11Device->CreateDepthStencilState(&dsDesc, &d3d11depthState);

			mDepthStateMap[depthState] = d3d11depthState;
		}

		return mDepthStateMap[depthState];
	}

	ID3D11RasterizerState* GetRasterizerState(RasterizerState & rasterizerState)
	{
		auto entry = mRasterizerStateMap.find(rasterizerState);
		if (entry == mRasterizerStateMap.end())
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

			desc.ScissorEnable = false;
			desc.DepthClipEnable = true;

			ID3D11RasterizerState* d3d11rasterizerState;

			mD3d11Device->CreateRasterizerState(&desc, &d3d11rasterizerState);
			mRasterizerStateMap[rasterizerState] = d3d11rasterizerState;
		}

		return mRasterizerStateMap[rasterizerState];
	}

	int mClientWidth;
	int mClientHeight;
	std::unique_ptr<Skybox> mSkyBox;

	HINSTANCE	mHInst;
	HWND		mHMainWnd;

	UINT		m4xMsaaQuality;

	ID3D11Device* mD3d11Device;
	ID3D11DeviceContext* mD3d11DeviceContext;
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D_DRIVER_TYPE mD3dDriverType;

	D3D11_VIEWPORT mScreenViewpot;
	D3D11_VIEWPORT mScreenSuperSampleViewpot;
	ID3D11RasterizerState* mWireFrameRS;
	ID3D11SamplerState* mSamplerState;
	ID3D11SamplerState* mHeightMapSamplerState;
	ID3D11SamplerState* mShadowSamplerState;
	ID3D11SamplerState* mBorderSamplerState;

	ID3D11DepthStencilState* mNoZWriteDSState;
	ID3D11BlendState* mAdditiveAlphaAddBS;

#pragma region DEDERRED_RENDER
	ID3D11RenderTargetView* mDiffuseBufferRTV;
	ID3D11RenderTargetView* mNormalBufferRTV;
	ID3D11RenderTargetView* mSpecularBufferRTV;
	ID3D11RenderTargetView* mPositionBufferRTV;
	ID3D11RenderTargetView* mMotionVecBufferRTV;
	ID3D11RenderTargetView* mEdgeBufferRTV;
	ID3D11RenderTargetView* mSSAORTV;
	ID3D11RenderTargetView* mSSAORTV2;
	ID3D11RenderTargetView* mDeferredShadingRTV;

	ID3D11ShaderResourceView* mRandVecTexSRV;

	ID3D11ShaderResourceView* mDiffuseBufferSRV;
	ID3D11ShaderResourceView* mNormalBufferSRV;
	ID3D11ShaderResourceView* mSpecularBufferSRV;
	ID3D11ShaderResourceView* mMotionVecBufferSRV;
	ID3D11ShaderResourceView* mEdgeBufferSRV;
	ID3D11ShaderResourceView* mSSAOSRV;
	ID3D11ShaderResourceView* mSSAOSRV2;
	ID3D11ShaderResourceView* mDeferredShadingSRV;

	ID3D11DepthStencilView* mDeferredRenderDepthStencilView;
	ID3D11ShaderResourceView* mDeferredRenderShaderResourceView;

#pragma endregion

	ID3D11ShaderResourceView* mOutputShaderResources[4];
	ID3D11UnorderedAccessView* mUnorderedAccessViews[4];

	bool mEnable4xMsaa;

	ID3D11Buffer* mGridCoordIndexBufferGPU;
	ID3D11Buffer* mGridCoordVertexBufferGPU;

	ID3D11RasterizerState* mDepthRS;

	std::vector<DrawCmd> mDrawList;

	std::vector<DrawEventHandler> mDrawHandler;
	std::vector<DrawEventHandler> mShadowDrawHandler;

	std::unordered_map<PrimitiveTopology, D3D_PRIMITIVE_TOPOLOGY> mDrawTopologyMap;
	std::unordered_map<BlendState, ID3D11BlendState*> mBlendStateMap;
	std::unordered_map<DepthState, ID3D11DepthStencilState*> mDepthStateMap;
	std::unordered_map<RasterizerState, ID3D11RasterizerState*> mRasterizerStateMap;

	RenderTarget mGBuffer;
	RenderTarget mDeferredShadingRT;
	RenderTarget mDefaultRTNoDepth;
	RenderTarget mDefaultRTWithDepth;
	RenderTarget mDebugRT;
};

Renderer* CreateD3D11Renderer(HINSTANCE hInstance, HWND hMainWnd)
{
	auto renderer = new D3D11Renderer(hInstance, hMainWnd);
	return static_cast<Renderer*>(renderer);
}

}
