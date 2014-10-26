#include "EffectsManager.h"
#include "D3D11Renderer.h"

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

Effect::~Effect()
{
	ReleaseCOM(m_fx);
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
	: Effect(filename)
{
	DeferredShaderTech = m_fx->GetTechniqueByName("DeferredShaderTech");
	WorldViewProj = m_fx->GetVariableByName("gWorldViewProj")->AsMatrix();
	WorldInvTranspose = m_fx->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	WorldView = m_fx->GetVariableByName("gWorldView")->AsMatrix();
	WorldViewInvTranspose = m_fx->GetVariableByName("gWorldViewInvTranspose")->AsMatrix();
	ShadowTransform = m_fx->GetVariableByName("gShadowTransform")->AsMatrix();
	World = m_fx->GetVariableByName("gWorld")->AsMatrix();
	//DirLight = m_fx->GetVariableByName("gDirLight");
	Mat = m_fx->GetVariableByName("gMaterial");
	EyePosW = m_fx->GetVariableByName("gEyePosW")->AsVector();
	DiffuseMap = m_fx->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap = m_fx->GetVariableByName("gNormalMap")->AsShaderResource();
	TheShadowMap = m_fx->GetVariableByName("gShadowMap")->AsShaderResource();

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	DeferredShaderTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 4, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_inputLayout));

	m_activeTech = DeferredShaderTech;
}

DeferredShaderEffect::~DeferredShaderEffect()
{
	ReleaseCOM(m_inputLayout);
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
EffectsManager* EffectsManager::_instance = nullptr;
EffectsManager::EffectsManager() {
	m_stdMeshEffect = new StdMeshEffect(L"FX/StdMesh.fxo");
	m_shadowMapEffect = new ShadowMapEffect(L"FX/ShadowMap.fxo");
	m_deferredShaderEffect = new DeferredShaderEffect(L"FX/DeferredShader.fxo");
	m_screenQuadEffect = new ScreenQuadEffect(L"FX/ScreenQuad.fxo");
	m_godrayEffect = new GodRayEffect(L"FX/GodRay.fxo");
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
}