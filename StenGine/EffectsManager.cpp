#include "EffectsManager.h"
#include "D3D11Renderer.h"
#include "D3DCompiler.h"

Effect::Effect(const std::wstring& filename)
	: m_fx(0)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, D3D11Renderer::Instance()->GetD3DDevice(), &m_fx));
}

Effect::Effect(const std::wstring& vsPath,
			   const std::wstring& psPath,
			   const std::wstring& gsPath = L"",
			   const std::wstring& hsPath = L"",
			   const std::wstring& dsPath = L""):
			   m_vertexShader(0),
			   m_pixelShader(0),
			   m_geometryShader(0),
			   m_hullShader(0),
			   m_domainShader(0),
			   m_vsBlob(0),
			   m_psBlob(0),
			   m_gsBlob(0),
			   m_hsBlob(0),
			   m_dsBlob(0)
{
	// Add error checking
	HRESULT hr;

	if (vsPath.length()) {
		hr = D3DCompileFromFile(
			vsPath.c_str(), 
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"vs_5_0",
			D3DCOMPILE_DEBUG,
			0,
			&m_vsBlob,
			nullptr
		);
		assert(SUCCEEDED(hr));
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateVertexShader(
			m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(),
			nullptr,
			&m_vertexShader
		);
		assert(SUCCEEDED(hr));
	}

	if (psPath.length()) {
		hr = D3DCompileFromFile(
			psPath.c_str(),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"ps_5_0",
			D3DCOMPILE_DEBUG,
			0,
			&m_psBlob,
			nullptr
		);
		assert(SUCCEEDED(hr));
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreatePixelShader(
			m_psBlob->GetBufferPointer(),
			m_psBlob->GetBufferSize(),
			nullptr,
			&m_pixelShader
		);
		assert(SUCCEEDED(hr));
	}
}

Effect::~Effect()
{
	ReleaseCOM(m_fx);
}

void Effect::UnBindConstantBuffer() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 0, nullptr);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 0, nullptr);
}

void Effect::UnBindShaderResource() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 0, nullptr);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 0, nullptr);
}

void Effect::SetShader() {
	// TODO: check if shader exists first
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShader(m_vertexShader, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShader(m_pixelShader, 0, 0);
}

void Effect::UnSetShader() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->HSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->GSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->DSSetShader(nullptr, 0, 0);
}
//------------------------------------------------------------//


StdMeshEffect::StdMeshEffect(const std::wstring& filename)
	: Effect(filename)
{
	StdMeshTech = m_fx->GetTechniqueByName("StdMeshTech");
	WorldViewProj = m_fx->GetVariableByName("gWorldViewProj")->AsMatrix();
	WorldInvTranspose = m_fx->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	ShadowTransform = m_fx->GetVariableByName("gShadowTransform")->AsMatrix();
	World = m_fx->GetVariableByName("gWorld")->AsMatrix();
	DirLight = m_fx->GetVariableByName("gDirLight");
	Mat = m_fx->GetVariableByName("gMaterial");
	EyePosW = m_fx->GetVariableByName("gEyePosW")->AsVector();
	DiffuseMap = m_fx->GetVariableByName("gDiffuseMap")->AsShaderResource();
	TheShadowMap = m_fx->GetVariableByName("gShadowMap")->AsShaderResource();

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	StdMeshTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 4, passDesc.pIAInputSignature,
	passDesc.IAInputSignatureSize, &m_inputLayout));

	m_activeTech = StdMeshTech;
}

StdMeshEffect::~StdMeshEffect()
{
	ReleaseCOM(m_inputLayout);
}


//----------------------------------------------------------//


ShadowMapEffect::ShadowMapEffect(const std::wstring& filename)
	: Effect(filename)
{
	ShadowMapTech = m_fx->GetTechniqueByName("BuildShadowMapTech");
	WorldViewProj = m_fx->GetVariableByName("gWorldViewProj")->AsMatrix();

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3DX11_PASS_DESC passDesc;
	ShadowMapTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_inputLayout));

	m_activeTech = ShadowMapTech;
}

ShadowMapEffect::~ShadowMapEffect()
{
	ReleaseCOM(m_inputLayout);
}


//------------------------------------------------------------//


DeferredShaderEffect::DeferredShaderEffect(const std::wstring& filename)
	: Effect(filename + L"_vs.hlsl", filename + L"_ps.hlsl")
{
// 	DeferredShaderTech = m_fx->GetTechniqueByName("DeferredShaderTech");
// 	DeferredShaderTessTech = m_fx->GetTechniqueByName("DeferredShaderTessTech");
// 	WorldViewProj = m_fx->GetVariableByName("gWorldViewProj")->AsMatrix();
// 	WorldInvTranspose = m_fx->GetVariableByName("gWorldInvTranspose")->AsMatrix();
// 	WorldView = m_fx->GetVariableByName("gWorldView")->AsMatrix();
// 	WorldViewInvTranspose = m_fx->GetVariableByName("gWorldViewInvTranspose")->AsMatrix();
// 	ShadowTransform = m_fx->GetVariableByName("gShadowTransform")->AsMatrix();
// 	World = m_fx->GetVariableByName("gWorld")->AsMatrix();
// 	ViewProj = m_fx->GetVariableByName("gViewProj")->AsMatrix();
// 	//DirLight = m_fx->GetVariableByName("gDirLight");
// 	Mat = m_fx->GetVariableByName("gMaterial");
// 	EyePosW = m_fx->GetVariableByName("gEyePosW")->AsVector();
// 	DiffuseMap = m_fx->GetVariableByName("gDiffuseMap")->AsShaderResource();
// 	BumpMap = m_fx->GetVariableByName("gBumpMap")->AsShaderResource();
// 	NormalMap = m_fx->GetVariableByName("gNormalMap")->AsShaderResource();
// 	TheShadowMap = m_fx->GetVariableByName("gShadowMap")->AsShaderResource();
// 	DiffX_NormY_ShadZ = m_fx->GetVariableByName("gDiffX_NormY_ShadZ")->AsVector();
// 	CubeMap = m_fx->GetVariableByName("gCubeMap")->AsShaderResource();


	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//D3DX11_PASS_DESC passDesc;
	//DeferredShaderTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 4, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_activeTech = DeferredShaderTech;

	for (int i = 0; i < 5; i++) {
		m_shaderResources[i] = 0;
	}
}

DeferredShaderEffect::~DeferredShaderEffect()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredShaderEffect::CreateConstantBuffer() {
	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perObjConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perFrameConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
			&m_perFrameCB);

		assert(SUCCEEDED(hr));
	}
}

void DeferredShaderEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 2, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 2, cbuf);

	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 2, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 2, cbuf);
}

void DeferredShaderEffect::BindShaderResource() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 5, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 5, m_shaderResources);
}
//----------------------------------------------------------//


ScreenQuadEffect::ScreenQuadEffect(const std::wstring& filename)
	: Effect(filename)
{
	FullScreenQuadTech = m_fx->GetTechniqueByName("t0");
//	SSAOTech = m_fx->GetTechniqueByName("SSAOTech");
	HBlurTech = m_fx->GetTechniqueByName("HBlurTech");
	VBlurTech = m_fx->GetTechniqueByName("VBlurTech");
	ScreenMap = m_fx->GetVariableByName("gScreenMap")->AsShaderResource();
	SSAOMap = m_fx->GetVariableByName("gSSAOMap")->AsShaderResource();
	DiffuseGB = m_fx->GetVariableByName("gDiffuseGB")->AsShaderResource();
	PositionGB = m_fx->GetVariableByName("gPositionGB")->AsShaderResource();
	SpecularGB = m_fx->GetVariableByName("gSpecularGB")->AsShaderResource();
	NormalGB = m_fx->GetVariableByName("gNormalGB")->AsShaderResource();
	DepthGB = m_fx->GetVariableByName("gDepthGB")->AsShaderResource();
	ProjInv = m_fx->GetVariableByName("gProjInv")->AsMatrix();
	Proj = m_fx->GetVariableByName("gProj")->AsMatrix();
	DirLight = m_fx->GetVariableByName("gDirLight");
	//Mat = m_fx->GetVariableByName("gMaterial");
	EyePosW = m_fx->GetVariableByName("gEyePosW")->AsVector();

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	FullScreenQuadTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_inputLayout));

	m_activeTech = FullScreenQuadTech;
}

ScreenQuadEffect::~ScreenQuadEffect()
{
	//ReleaseCOM(m_inputLayout);
}


//----------------------------------------------------------//


GodRayEffect::GodRayEffect(const std::wstring& filename)
	: Effect(filename)
{
	GodRayTech = m_fx->GetTechniqueByName("t0");
	OcclusionMap = m_fx->GetVariableByName("gOcclusionMap")->AsShaderResource();
	LightPosH = m_fx->GetVariableByName("gLightPosH")->AsVector();

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	GodRayTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_inputLayout));

	m_activeTech = GodRayTech;
}

GodRayEffect::~GodRayEffect()
{
	//ReleaseCOM(m_inputLayout);
}


//----------------------------------------------------------//


SkyboxEffect::SkyboxEffect(const std::wstring& filename)
	: Effect(filename)
{
	SkyboxTech = m_fx->GetTechniqueByName("SkyboxTech");
	WorldViewProj = m_fx->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap = m_fx->GetVariableByName("gCubeMap")->AsShaderResource();

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3DX11_PASS_DESC passDesc;
	SkyboxTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_inputLayout));

	m_activeTech = SkyboxTech;
}

SkyboxEffect::~SkyboxEffect()
{
	ReleaseCOM(m_inputLayout);
}



//----------------------------------------------------------//
EffectsManager* EffectsManager::_instance = nullptr;
EffectsManager::EffectsManager() {
	//m_stdMeshEffect = new StdMeshEffect(L"FX/StdMesh.fxo");
	//m_shadowMapEffect = new ShadowMapEffect(L"FX/ShadowMap.fxo");
	m_deferredShaderEffect = new DeferredShaderEffect(L"FX/DeferredShader");
	//m_screenQuadEffect = new ScreenQuadEffect(L"FX/ScreenQuad.fxo");
	//m_godrayEffect = new GodRayEffect(L"FX/GodRay.fxo");
	//m_skyboxEffect = new SkyboxEffect(L"FX/Skybox.fxo");
	//m_effects.push_back(PosColor);
}

EffectsManager::~EffectsManager() {
	//for (int i = 0; i < m_effects.size(); i++) {
	//	delete m_effects[i];
	//}
	delete m_stdMeshEffect;
	delete m_shadowMapEffect;
	delete m_deferredShaderEffect;
	delete m_screenQuadEffect;
	delete m_godrayEffect;
	delete m_skyboxEffect;
}