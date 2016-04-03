#include "Skybox.h"
#include "RendererBase.h"
#include "CameraManager.h"
#include "MeshRenderer.h"

#ifdef GRAPHICS_OPENGL
#include "GLImageLoader.h"
#endif

Skybox::Skybox(std::wstring &cubeMapPath) {
// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice()), 
// 		cubeMapPath.c_str(), 0, 0, &m_cubeMapSRV, 0));
#ifdef GRAPHICS_D3D11
	CreateDDSTextureFromFile(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice()),
		cubeMapPath.c_str(), nullptr, &m_cubeMapSRV);
#else
	std::string s(cubeMapPath.begin(), cubeMapPath.end());
	GLuint cubemap = CreateGLTextureFromFile(s.c_str());
	assert(cubemap != 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_cubeMapTex = glGetTextureHandleARB(cubemap);
	glMakeTextureHandleResidentARB(m_cubeMapTex);

	// generate VAO
	std::vector<XMFLOAT3> skyboxVertexBuffer = {
		XMFLOAT3(-1.0f, -1.0f, -1.0f),
		XMFLOAT3(-1.0f, +1.0f, -1.0f),
		XMFLOAT3(+1.0f, +1.0f, -1.0f),
		XMFLOAT3(+1.0f, -1.0f, -1.0f),
		XMFLOAT3(-1.0f, -1.0f, +1.0f),
		XMFLOAT3(+1.0f, -1.0f, +1.0f),
		XMFLOAT3(+1.0f, +1.0f, +1.0f),
		XMFLOAT3(-1.0f, +1.0f, +1.0f),
		XMFLOAT3(-1.0f, +1.0f, -1.0f),
		XMFLOAT3(-1.0f, +1.0f, +1.0f),
		XMFLOAT3(+1.0f, +1.0f, +1.0f),
		XMFLOAT3(+1.0f, +1.0f, -1.0f),
		XMFLOAT3(-1.0f, -1.0f, -1.0f),
		XMFLOAT3(+1.0f, -1.0f, -1.0f),
		XMFLOAT3(+1.0f, -1.0f, +1.0f),
		XMFLOAT3(-1.0f, -1.0f, +1.0f),
		XMFLOAT3(-1.0f, -1.0f, +1.0f),
		XMFLOAT3(-1.0f, +1.0f, +1.0f),
		XMFLOAT3(-1.0f, +1.0f, -1.0f),
		XMFLOAT3(-1.0f, -1.0f, -1.0f),
		XMFLOAT3(+1.0f, -1.0f, -1.0f),
		XMFLOAT3(+1.0f, +1.0f, -1.0f),
		XMFLOAT3(+1.0f, +1.0f, +1.0f),
		XMFLOAT3(+1.0f, -1.0f, +1.0f),
	};

	std::vector<UINT> skyboxIndexBuffer = {
		1, 0, 2,
		2, 0, 3,

		5, 4, 6,
		6, 4, 7,

		9, 8, 10,
		10, 8, 11,

		13, 12, 14,
		14, 12, 15,

		17, 16, 18,
		18, 16, 19,

		21, 20, 22,
		22, 20, 23
	};

	GLuint skyboxVertexVBO;
	GLuint skyboxIndexVBO;
	glGenBuffers(1, &skyboxVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, skyboxVertexBuffer.size() * sizeof(XMFLOAT3), &skyboxVertexBuffer[0], GL_STATIC_DRAW);

	glGenBuffers(1, &skyboxIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, skyboxIndexBuffer.size() * sizeof(UINT), &skyboxIndexBuffer[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_skyboxVAO);
	glBindVertexArray(m_skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVertexVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndexVBO);
#endif
}

Skybox::~Skybox() {
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_cubeMapSRV);
#else
	glMakeTextureHandleNonResidentARB(m_cubeMapTex);
#endif
}

void Skybox::Draw() {
	SkyboxEffect* skyboxEffect = EffectsManager::Instance()->m_skyboxEffect;
	skyboxEffect->SetShader();
#ifdef GRAPHICS_D3D11
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//skyboxEffect->m_shaderResources[0] = m_cubeMapSRV;
	skyboxEffect->SetShaderResources(m_cubeMapSRV, 0);
	

	XMFLOAT4 eyePos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	skyboxEffect->m_perObjConstantBuffer.gWorldViewProj = XMMatrixTranspose(WVP);

	skyboxEffect->UpdateConstantBuffer();
	skyboxEffect->BindConstantBuffer();
	skyboxEffect->BindShaderResource();
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Draw(36, 0);

#else
	glBindVertexArray(m_skyboxVAO);
	skyboxEffect->CubeMap = m_cubeMapTex;
	XMFLOAT4 eyePos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	skyboxEffect->m_perObjUniformBuffer.gWorldViewProj = WVP;
	skyboxEffect->UpdateConstantBuffer();

	//glDrawArrays(GL_TRIANGLES, 0, 36);

	glDrawElements(
		GL_TRIANGLES,      // mode
		36,    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

#endif
	skyboxEffect->UnBindShaderResource();
	skyboxEffect->UnBindConstantBuffer();

	skyboxEffect->UnSetShader();

}