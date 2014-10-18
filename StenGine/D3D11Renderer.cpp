#include "D3D11Renderer.h"
#include "D3DIncludes.h"
#include "EffectsManager.h"
#include "MeshRenderer.h"
#include "LightManager.h"
#include "MathHelper.h"
#include "CameraManager.h"

D3D11Renderer* D3D11Renderer::_instance = nullptr;

D3D11Renderer::D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd):
m_hInst(hInstance),
m_hMainWnd(hMainWnd),
m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
m_clientWidth(1280),
m_clientHeight(720),
m_enable4xMsaa(true),
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


	m_screenViewpot.TopLeftX = 0;
	m_screenViewpot.TopLeftY = 0;
	m_screenViewpot.Width = static_cast<float>(m_clientWidth);
	m_screenViewpot.Height = static_cast<float>(m_clientHeight);
	m_screenViewpot.MinDepth = 0.0f;
	m_screenViewpot.MaxDepth = 1.0f;


	//-----------------setup MRT---------------------
#if !FORWARD
	D3D11_TEXTURE2D_DESC gBufferDesc;
	gBufferDesc.Width = m_clientWidth;
	gBufferDesc.Height = m_clientHeight;
	gBufferDesc.MipLevels = 1;
	gBufferDesc.ArraySize = 1;
	gBufferDesc.SampleDesc.Count = 1;
	gBufferDesc.SampleDesc.Quality = 0;
	gBufferDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	gBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	gBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	gBufferDesc.CPUAccessFlags = 0;
	gBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* gBufferDTex = 0;
	ID3D11Texture2D* gBufferPTex = 0;
	ID3D11Texture2D* gBufferNTex = 0;
	ID3D11Texture2D* gBufferSTex = 0;
	HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferDTex));
	HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferPTex));
	HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferNTex));
	HR(m_d3d11Device->CreateTexture2D(&gBufferDesc, 0, &gBufferSTex));


	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = gBufferDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	HR(m_d3d11Device->CreateRenderTargetView(gBufferDTex, &rtvDesc, &m_diffuseBufferRTV));
	HR(m_d3d11Device->CreateRenderTargetView(gBufferPTex, &rtvDesc, &m_positionBufferRTV));
	HR(m_d3d11Device->CreateRenderTargetView(gBufferNTex, &rtvDesc, &m_normalBufferRTV));
	HR(m_d3d11Device->CreateRenderTargetView(gBufferSTex, &rtvDesc, &m_specularBufferRTV));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = gBufferDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	HR(m_d3d11Device->CreateShaderResourceView(gBufferDTex, &srvDesc, &m_diffuseBufferSRV));
	HR(m_d3d11Device->CreateShaderResourceView(gBufferPTex, &srvDesc, &m_positionBufferSRV));
	HR(m_d3d11Device->CreateShaderResourceView(gBufferNTex, &srvDesc, &m_normalBufferSRV));
	HR(m_d3d11Device->CreateShaderResourceView(gBufferSTex, &srvDesc, &m_specularBufferSRV));

	ReleaseCOM(gBufferDTex);
	ReleaseCOM(gBufferPTex);
	ReleaseCOM(gBufferNTex);
	ReleaseCOM(gBufferSTex);


	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = m_clientWidth;
	depthTexDesc.Height = m_clientHeight;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	HR(m_d3d11Device->CreateTexture2D(&depthTexDesc, 0, &depthTex));

	// Create the depth stencil view for the entire cube
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(m_d3d11Device->CreateDepthStencilView(depthTex, &dsvDesc, &m_deferredRenderDepthStencilView));

	ReleaseCOM(depthTex);

#endif

	DirectionalLight* dLight = new DirectionalLight();
	dLight->intensity = XMFLOAT4(1, 1, 1, 1);
	dLight->direction = MatrixHelper::NormalizeFloat3(XMFLOAT3(-0.5, -2, 1));
	dLight->castShadow = 1;

	LightManager::Instance()->m_dirLights.push_back(dLight);
	LightManager::Instance()->m_shadowMap = new ShadowMap(1024, 1024);

	return true;
}

void D3D11Renderer::Draw() {

	LightManager::Instance()->m_shadowMap->RenderShadowMap();

 	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
 	m_d3d11DeviceContext->RSSetViewports(1, &m_screenViewpot);


#if FORWARD
	m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_d3d11DeviceContext->RSSetState(0);
		StdMeshEffect* activeFX = EffectsManager::Instance()->m_stdMeshEffect;
		D3D11Renderer::Instance()->GetD3DContext()->IASetInputLayout(activeFX->GetInputLayout());
		D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DirectionalLight* d = LightManager::Instance()->m_dirLights[0];
		activeFX->DirLight->SetRawValue(LightManager::Instance()->m_dirLights[0], 0, sizeof(DirectionalLight));
		activeFX->TheShadowMap->SetResource(LightManager::Instance()->m_shadowMap->GetDepthSRV());

		XMFLOAT4 pos = CameraManager::Instance()->GetActiveCamera()->GetPos();
		activeFX->EyePosW->SetRawValue(&pos, 0, 3 * sizeof(float));

		for (int iMesh = 0; iMesh < activeFX->m_associatedMeshes.size(); iMesh++ ) {
			activeFX->m_associatedMeshes[iMesh]->Draw();
		}
#else

	ID3D11RenderTargetView* rtvs[4] = { m_diffuseBufferRTV, m_positionBufferRTV, m_normalBufferRTV, m_specularBufferRTV };
	m_d3d11DeviceContext->OMSetRenderTargets(4, rtvs, m_deferredRenderDepthStencilView);
	//m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3d11DeviceContext->ClearRenderTargetView(m_diffuseBufferRTV, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3d11DeviceContext->ClearRenderTargetView(m_positionBufferRTV, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3d11DeviceContext->ClearRenderTargetView(m_normalBufferRTV, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3d11DeviceContext->ClearRenderTargetView(m_specularBufferRTV, reinterpret_cast<const float*>(&Colors::Black));
	m_d3d11DeviceContext->ClearDepthStencilView(m_deferredRenderDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_d3d11DeviceContext->RSSetState(0);
		DeferredShaderEffect* activeFX = EffectsManager::Instance()->m_deferredShaderEffect;
		m_d3d11DeviceContext->IASetInputLayout(activeFX->GetInputLayout());
		m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DirectionalLight* d = LightManager::Instance()->m_dirLights[0];
		//activeFX->DirLight->SetRawValue(LightManager::Instance()->m_dirLights[0], 0, sizeof(DirectionalLight));
		activeFX->TheShadowMap->SetResource(LightManager::Instance()->m_shadowMap->GetDepthSRV());

		XMFLOAT4 pos = CameraManager::Instance()->GetActiveCamera()->GetPos();
		activeFX->EyePosW->SetRawValue(&pos, 0, 3 * sizeof(float));

		for (int iMesh = 0; iMesh < EffectsManager::Instance()->m_stdMeshEffect->m_associatedMeshes.size(); iMesh++) {
			EffectsManager::Instance()->m_stdMeshEffect->m_associatedMeshes[iMesh]->Draw();
		}


	//m_d3d11DeviceContext->RSSetState(0);
	//m_d3d11DeviceContext->OMSetDepthStencilState(0, 0);

	m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	ID3DX11EffectTechnique* screenQuadTech = EffectsManager::Instance()->m_screenQuadEffect->GetActiveTech();
	m_d3d11DeviceContext->IASetInputLayout(NULL);
	screenQuadTech->GetDesc(&techDesc);

	EffectsManager::Instance()->m_screenQuadEffect->DirLight->SetRawValue(LightManager::Instance()->m_dirLights[0], 0, sizeof(DirectionalLight));

	XMFLOAT4 camPos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	EffectsManager::Instance()->m_screenQuadEffect->EyePosW->SetRawValue(&camPos, 0, 3 * sizeof(float));

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{

		EffectsManager::Instance()->m_screenQuadEffect->DiffuseGB->SetResource(m_diffuseBufferSRV);
		EffectsManager::Instance()->m_screenQuadEffect->PositionGB->SetResource(m_positionBufferSRV);
		EffectsManager::Instance()->m_screenQuadEffect->NormalGB->SetResource(m_normalBufferSRV);
		EffectsManager::Instance()->m_screenQuadEffect->SpecularGB->SetResource(m_specularBufferSRV);

		screenQuadTech->GetPassByIndex(p)->Apply(0, m_d3d11DeviceContext);
		m_d3d11DeviceContext->Draw(6, 0);
	}


	m_d3d11DeviceContext->RSSetState(0);
	m_d3d11DeviceContext->OMSetDepthStencilState(0, 0);


#endif



//	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	m_d3d11DeviceContext->PSSetShaderResources(0, 16, nullSRV);

	HR(m_swapChain->Present(0, 0));
}
