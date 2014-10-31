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
	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);
	ReleaseCOM(m_geometryShader);
	ReleaseCOM(m_hullShader);
	ReleaseCOM(m_domainShader);
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
	if (m_vertexShader)
		D3D11Renderer::Instance()->GetD3DContext()->VSSetShader(m_vertexShader, 0, 0);
	if (m_pixelShader)
		D3D11Renderer::Instance()->GetD3DContext()->PSSetShader(m_pixelShader, 0, 0);
	if (m_geometryShader)
		D3D11Renderer::Instance()->GetD3DContext()->GSSetShader(m_geometryShader, 0, 0);
	if (m_hullShader)
		D3D11Renderer::Instance()->GetD3DContext()->HSSetShader(m_hullShader, 0, 0);
	if (m_domainShader)
		D3D11Renderer::Instance()->GetD3DContext()->DSSetShader(m_domainShader, 0, 0);
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


DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs.hlsl", filename + L"_ps.hlsl")
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 4, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	for (int i = 0; i < 5; i++) {
		m_shaderResources[i] = 0;
	}

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

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

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
}

DeferredGeometryPassEffect::~DeferredGeometryPassEffect()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredGeometryPassEffect::UpdateConstantBuffer() {
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perObjectCB, NULL);
	}

	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perFrameCB, NULL);
	}
}

void DeferredGeometryPassEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 2, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 2, cbuf);
}

void DeferredGeometryPassEffect::BindShaderResource() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 5, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 5, m_shaderResources);
}


//----------------------------------------------------------//


DeferredShadingPassEffect::DeferredShadingPassEffect(const std::wstring& filename)
	: Effect(L"FX/ScreenQuad_vs.hlsl", filename + L"_ps.hlsl")
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	for (int i = 0; i < 4; i++) {
		m_shaderResources[i] = 0;
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

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
}

DeferredShadingPassEffect::~DeferredShadingPassEffect()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredShadingPassEffect::UpdateConstantBuffer() {
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perFrameCB, NULL);
	}
}

void DeferredShadingPassEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perFrameCB };
	//D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 1, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 1, cbuf);
}

void DeferredShadingPassEffect::BindShaderResource() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 4, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 4, m_shaderResources);
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
	m_deferredGeometryPassEffect = new DeferredGeometryPassEffect(L"FX/DeferredGeometryPass");
	m_deferredShadingPassEffect = new DeferredShadingPassEffect(L"FX/DeferredShadingPass");

	//m_screenQuadEffect = new ScreenQuadEffect(L"FX/ScreenQuad.fxo");
	//m_godrayEffect = new GodRayEffect(L"FX/GodRay.fxo");
	//m_skyboxEffect = new SkyboxEffect(L"FX/Skybox.fxo");
	//m_effects.push_back(PosColor);
}

EffectsManager::~EffectsManager() {
	//for (int i = 0; i < m_effects.size(); i++) {
	//	delete m_effects[i];
	//}
	SafeDelete(m_stdMeshEffect);
	SafeDelete(m_shadowMapEffect);
	SafeDelete(m_deferredGeometryPassEffect);
	SafeDelete(m_deferredShadingPassEffect);
	SafeDelete(m_screenQuadEffect);
	SafeDelete(m_godrayEffect);
	SafeDelete(m_skyboxEffect);
}