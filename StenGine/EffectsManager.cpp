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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	StdMeshTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 3, passDesc.pIAInputSignature,
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


//----------------------------------------------------------//


EffectsManager* EffectsManager::_instance = nullptr;
EffectsManager::EffectsManager() {
	m_stdMeshEffect = new StdMeshEffect(L"FX/StdMesh.fxo");
	m_shadowMapEffect = new ShadowMapEffect(L"FX/ShadowMap.fxo");
	//m_effects.push_back(PosColor);
}

EffectsManager::~EffectsManager() {
	//for (int i = 0; i < m_effects.size(); i++) {
	//	delete m_effects[i];
	//}
	delete m_stdMeshEffect;
	delete m_shadowMapEffect;
}