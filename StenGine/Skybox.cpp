#include "Skybox.h"
#include "D3D11Renderer.h"
#include "CameraManager.h"

Skybox::Skybox(std::wstring &cubeMapPath) {
// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		D3D11Renderer::Instance()->GetD3DDevice(), 
// 		cubeMapPath.c_str(), 0, 0, &m_cubeMapSRV, 0));
	CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
		cubeMapPath.c_str(), nullptr, &m_cubeMapSRV);
}

Skybox::~Skybox() {
	ReleaseCOM(m_cubeMapSRV);
}

void Skybox::Draw() {
	D3DX11_TECHNIQUE_DESC techDesc;
	ID3DX11EffectTechnique* SkyboxTech = EffectsManager::Instance()->m_skyboxEffect->SkyboxTech;
	SkyboxTech->GetDesc(&techDesc);
	EffectsManager::Instance()->m_skyboxEffect->CubeMap->SetResource(m_cubeMapSRV);

	XMFLOAT4 eyePos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
	EffectsManager::Instance()->m_skyboxEffect->WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&WVP));


	//EffectsManager::Instance()->m_skyboxEffect->ScreenMap->SetResource(m_deferredShadingSRV);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		SkyboxTech->GetPassByIndex(p)->Apply(0, D3D11Renderer::Instance()->GetD3DContext());
		D3D11Renderer::Instance()->GetD3DContext()->Draw(36, 0);
	}

}