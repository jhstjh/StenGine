#include "Graphics/Effect/Skybox.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Scene/CameraManager.h"
#include "Mesh/MeshRenderer.h"
#include "Math/MathHelper.h"

#if GRAPHICS_OPENGL
#include "Graphics/OpenGL/GLImageLoader.h"
#endif

namespace StenGine
{

Skybox::Skybox(std::wstring &cubeMapPath) {
#if GRAPHICS_D3D11
	CreateDDSTextureFromFile(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice()),
		cubeMapPath.c_str(), nullptr, &m_cubeMapSRV);
#else
	std::string s(cubeMapPath.begin(), cubeMapPath.end());
	GLuint cubemap = CreateGLTextureFromFile(s.c_str());
	assert(cubemap != 0);
	glTextureParameteri(cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(cubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
	glCreateBuffers(1, &skyboxVertexVBO);
	glNamedBufferStorage(skyboxVertexVBO, skyboxVertexBuffer.size() * sizeof(XMFLOAT3), &skyboxVertexBuffer[0], 0);

	glCreateBuffers(1, &skyboxIndexVBO);
	glNamedBufferStorage(skyboxIndexVBO, skyboxIndexBuffer.size() * sizeof(UINT), &skyboxIndexBuffer[0], 0);

	glCreateVertexArrays(1, &m_skyboxVAO);

	glEnableVertexArrayAttrib(m_skyboxVAO, 0);
	glVertexArrayAttribFormat(m_skyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(m_skyboxVAO, 0, skyboxVertexVBO, 0, sizeof(XMFLOAT3));
	glVertexArrayAttribBinding(m_skyboxVAO, 0, 0);
	glVertexArrayElementBuffer(m_skyboxVAO, skyboxIndexVBO);

#endif
}

Skybox::~Skybox() {
#if GRAPHICS_D3D11
	ReleaseCOM(m_cubeMapSRV);
#else
	glMakeTextureHandleNonResidentARB(m_cubeMapTex);
#endif
}

void Skybox::Draw() {
	SkyboxEffect* skyboxEffect = EffectsManager::Instance()->m_skyboxEffect.get();

	ConstantBuffer cbuffer0(0, sizeof(SkyboxEffect::PEROBJ_CONSTANT_BUFFER), (void*)skyboxEffect->m_perObjectCB->GetBuffer());
	SkyboxEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (SkyboxEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0.GetBuffer();

	XMFLOAT4 eyePos = CameraManager::Instance()->GetActiveCamera()->GetPos();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	perObjData->gWorldViewProj = TRASNPOSE_API_CHOOSER(WVP);

	DrawCmd cmd;
	
	cmd.flags = CmdFlag::DRAW;
	cmd.type = PrimitiveTopology::TRIANGLELIST;
	cmd.indexBuffer = 0;
	cmd.vertexBuffer = 0;
#if GRAPHICS_OPENGL
	cmd.drawType = DrawType::INDEXED;
	cmd.inputLayout = (void*)m_skyboxVAO;
	perObjData->gCubeMap = m_cubeMapTex;
#endif
	cmd.offset = 0;
	cmd.effect = skyboxEffect;
	cmd.elementCount = 36;
	cmd.cbuffers.push_back(std::move(cbuffer0));

#if GRAPHICS_D3D11
	cmd.drawType = DrawType::ARRAY;
	cmd.srvs.AddSRV(m_cubeMapSRV, 0);
	cmd.inputLayout = skyboxEffect->GetInputLayout();
#endif

	Renderer::Instance()->AddDeferredDrawCmd(cmd);
}

}