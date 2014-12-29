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
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_MIPMAPS
	);
	assert(m_cubeMapTex != 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
}

Skybox::~Skybox() {
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_cubeMapSRV);
#endif
}

void Skybox::Draw() {

	SkyboxEffect* skyboxEffect = EffectsManager::Instance()->m_skyboxEffect;
	skyboxEffect->SetShader();
#ifdef GRAPHICS_D3D11
	//skyboxEffect->m_shaderResources[0] = m_cubeMapSRV;
	skyboxEffect->SetShaderResources(m_cubeMapSRV, 0);
	

	XMFLOAT4 eyePos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	skyboxEffect->m_perObjConstantBuffer.gWorldViewProj = XMMatrixTranspose(WVP);

	skyboxEffect->UpdateConstantBuffer();
	skyboxEffect->BindConstantBuffer();
	skyboxEffect->BindShaderResource();
	D3D11Renderer::Instance()->GetD3DContext()->Draw(36, 0);

#else
	glBindVertexArray(NULL);
	skyboxEffect->CubeMap = m_cubeMapTex;
	XMFLOAT4 eyePos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	skyboxEffect->m_perObjUniformBuffer.gWorldViewProj = WVP;
	skyboxEffect->UpdateConstantBuffer();

	glDrawArrays(GL_TRIANGLES, 0, 36);


#endif
	skyboxEffect->UnBindShaderResource();
	skyboxEffect->UnBindConstantBuffer();

	skyboxEffect->UnSetShader();

}