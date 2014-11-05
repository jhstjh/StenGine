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

	SkyboxEffect* skyboxEffect = EffectsManager::Instance()->m_skyboxEffect;
	//skyboxEffect->m_shaderResources[0] = m_cubeMapSRV;
	skyboxEffect->SetShaderResources(m_cubeMapSRV, 0);
	skyboxEffect->SetShader();

	XMFLOAT4 eyePos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	skyboxEffect->m_perObjConstantBuffer.gWorldViewProj = XMMatrixTranspose(WVP);

	skyboxEffect->UpdateConstantBuffer();
	skyboxEffect->BindConstantBuffer();
	skyboxEffect->BindShaderResource();
	D3D11Renderer::Instance()->GetD3DContext()->Draw(36, 0);
	skyboxEffect->UnBindShaderResource();
	skyboxEffect->UnBindConstantBuffer();

	skyboxEffect->UnSetShader();

}