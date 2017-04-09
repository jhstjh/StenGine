#include "Graphics/Effect/Skybox.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Scene/CameraManager.h"
#include "Mesh/MeshRenderer.h"
#include "Math/MathDefs.h"
#include "Math/MathHelper.h"

#include "Graphics/OpenGL/GLImageLoader.h"

namespace StenGine
{

Skybox::Skybox(std::wstring &cubeMapPath) {
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		DirectX::CreateDDSTextureFromFile(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice()),
			cubeMapPath.c_str(), nullptr, &m_cubeMapSRV);
		break;
	}
	case RenderBackend::OPENGL4:
	{

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
		std::vector<Vec3Packed> skyboxVertexBuffer = {
			Vec3Packed({-1.0f, -1.0f, -1.0f}),
			Vec3Packed({-1.0f, +1.0f, -1.0f}),
			Vec3Packed({+1.0f, +1.0f, -1.0f}),
			Vec3Packed({+1.0f, -1.0f, -1.0f}),
			Vec3Packed({-1.0f, -1.0f, +1.0f}),
			Vec3Packed({+1.0f, -1.0f, +1.0f}),
			Vec3Packed({+1.0f, +1.0f, +1.0f}),
			Vec3Packed({-1.0f, +1.0f, +1.0f}),
			Vec3Packed({-1.0f, +1.0f, -1.0f}),
			Vec3Packed({-1.0f, +1.0f, +1.0f}),
			Vec3Packed({+1.0f, +1.0f, +1.0f}),
			Vec3Packed({+1.0f, +1.0f, -1.0f}),
			Vec3Packed({-1.0f, -1.0f, -1.0f}),
			Vec3Packed({+1.0f, -1.0f, -1.0f}),
			Vec3Packed({+1.0f, -1.0f, +1.0f}),
			Vec3Packed({-1.0f, -1.0f, +1.0f}),
			Vec3Packed({-1.0f, -1.0f, +1.0f}),
			Vec3Packed({-1.0f, +1.0f, +1.0f}),
			Vec3Packed({-1.0f, +1.0f, -1.0f}),
			Vec3Packed({-1.0f, -1.0f, -1.0f}),
			Vec3Packed({+1.0f, -1.0f, -1.0f}),
			Vec3Packed({+1.0f, +1.0f, -1.0f}),
			Vec3Packed({+1.0f, +1.0f, +1.0f}),
			Vec3Packed({+1.0f, -1.0f, +1.0f}),
		};

		std::vector<uint32_t> skyboxIndexBuffer = {
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
		glNamedBufferStorage(skyboxVertexVBO, skyboxVertexBuffer.size() * sizeof(Vec3Packed), &skyboxVertexBuffer[0], 0);

		glCreateBuffers(1, &skyboxIndexVBO);
		glNamedBufferStorage(skyboxIndexVBO, skyboxIndexBuffer.size() * sizeof(uint32_t), &skyboxIndexBuffer[0], 0);

		glCreateVertexArrays(1, &m_skyboxVAO);

		glEnableVertexArrayAttrib(m_skyboxVAO, 0);
		glVertexArrayAttribFormat(m_skyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayVertexBuffer(m_skyboxVAO, 0, skyboxVertexVBO, 0, sizeof(Vec3Packed));
		glVertexArrayAttribBinding(m_skyboxVAO, 0, 0);
		glVertexArrayElementBuffer(m_skyboxVAO, skyboxIndexVBO);

		break;
	}
	}
}

Skybox::~Skybox() {
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_cubeMapSRV);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		glMakeTextureHandleNonResidentARB(m_cubeMapTex);
		break;
	}
	}
}

void Skybox::Draw() {
	SkyboxEffect* skyboxEffect = EffectsManager::Instance()->m_skyboxEffect.get();

	ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(SkyboxEffect::PEROBJ_CONSTANT_BUFFER), skyboxEffect->m_perObjectCB);
	SkyboxEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (SkyboxEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0->GetBuffer();

	Mat4 T = Mat4::FromTranslationVector(CameraManager::Instance()->GetActiveCamera()->GetPos().xyz());
	Mat4 WVP = CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix() * T;

	perObjData->gWorldViewProj = TRASNPOSE_API_CHOOSER(WVP);

	DrawCmd cmd;
	
	cmd.flags = CmdFlag::DRAW;
	cmd.type = PrimitiveTopology::TRIANGLELIST;
	cmd.indexBuffer = 0;
	cmd.vertexBuffer = 0;

	if (Renderer::GetRenderBackend() == RenderBackend::OPENGL4)
	{
		cmd.drawType = DrawType::INDEXED;
		cmd.inputLayout = (void*)m_skyboxVAO;
		
		ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(SkyboxEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), skyboxEffect->m_textureCB);
		SkyboxEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (SkyboxEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

		textureData->gCubeMap = m_cubeMapTex;

		cmd.cbuffers.push_back(std::move(cbuffer1));
	}

	cmd.offset = 0;
	cmd.effect = skyboxEffect;
	cmd.elementCount = 36;
	cmd.cbuffers.push_back(std::move(cbuffer0));

	if (Renderer::GetRenderBackend() == RenderBackend::D3D11)
	{
		cmd.drawType = DrawType::ARRAY;
		cmd.srvs.AddSRV(m_cubeMapSRV, 0);
		cmd.inputLayout = skyboxEffect->GetInputLayout();
	}

	Renderer::Instance()->AddDeferredDrawCmd(cmd);
}

}