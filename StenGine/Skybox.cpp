#include "Skybox.h"
#include "D3D11Renderer.h"
#include "CameraManager.h"
#include "MeshRenderer.h"
#include "SOIL.h"

Skybox::Skybox(std::wstring &cubeMapPath) {
// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		D3D11Renderer::Instance()->GetD3DDevice(), 
// 		cubeMapPath.c_str(), 0, 0, &m_cubeMapSRV, 0));
#ifdef GRAPHICS_D3D11
	CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
		cubeMapPath.c_str(), nullptr, &m_cubeMapSRV);
#else
	std::string s(cubeMapPath.begin(), cubeMapPath.end());
	m_cubeMapTex = SOIL_load_OGL_single_cubemap
		(
		s.c_str(),
		SOIL_DDS_CUBEMAP_FACE_ORDER,
		SOIL_LOAD_AUTO,
		m_cubeMapTex,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_LOAD_DIRECT
	);
	assert(m_cubeMapTex != 0);
#endif
}

Skybox::~Skybox() {
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_cubeMapSRV);
#endif
}

void Skybox::Draw() {
#ifdef GRAPHICS_D3D11
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
#else
	// gl render
#endif

}