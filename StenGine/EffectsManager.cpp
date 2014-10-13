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

PosColorEffect::PosColorEffect(const std::wstring& filename)
	: Effect(filename)
{
	PosColorTech = m_fx->GetTechniqueByName("ColorTech");
	WorldViewProj = m_fx->GetVariableByName("gWorldViewProj")->AsMatrix();

	//m_vertexDesc.resize(2);
	//m_vertexDesc =
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	PosColorTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
	passDesc.IAInputSignatureSize, &m_inputLayout));

	m_activeTech = PosColorTech;
}

PosColorEffect::~PosColorEffect()
{
	ReleaseCOM(m_inputLayout);
}

EffectsManager* EffectsManager::_instance = nullptr;
EffectsManager::EffectsManager() {
	PosColorEffect* PosColor = new PosColorEffect(L"FX/Color.fxo");
	m_effects.push_back(PosColor);
}

EffectsManager::~EffectsManager() {
	for (int i = 0; i < m_effects.size(); i++) {
		delete m_effects[i];
	}
}