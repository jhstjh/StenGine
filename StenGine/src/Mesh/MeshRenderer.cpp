#include <algorithm>
#include <imgui.h>
#include "Math/MathHelper.h"
#include "Mesh/Mesh.h"
#include "Mesh/MeshRenderer.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Resource/ResourceManager.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/GameObject.h"
#include "Utility/ObjReader.h"

#pragma warning(disable: 4312) // 'type cast': conversion from 'GLuint' to 'void *' of greater size
#pragma warning(disable: 4267) // conversion from 'size_t' to 'UINT', possible loss of data
#pragma warning(disable: 4996)

namespace StenGine
{

MeshRenderer::~MeshRenderer()
{

}

void MeshRenderer::SetMesh(Mesh* mesh)
{
	mMesh = mesh;
	assert(mMesh != nullptr);
	Prepare();
}

void MeshRenderer::Prepare() 
{
	PrepareGPUBuffer();
	PrepareShadowMapBuffer();
}

void MeshRenderer::DrawMenu()
{
	if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Cast Shadow", &m_castShadow);
		ImGui::Checkbox("Receive Shadow", &m_receiveShadow);

		if (ImGui::TreeNode("Materials"))
		{
			char scratch[32];
			for (size_t i = 0; i < mMesh->m_materials.size(); ++i)
			{
				sprintf(scratch, "Material%zd", i);
				if (ImGui::TreeNode(scratch))
				{
					ImGui::DragFloat3("Diffuse", reinterpret_cast<float*>(&mMesh->m_materials[i].m_attributes.diffuse), 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat3("Roughness/Metallic/c/DoubleSided", reinterpret_cast<float*>(&mMesh->m_materials[i].m_attributes.roughness_metalic_c_doublesided), 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("DoubleSided", &mMesh->m_materials[i].m_attributes.roughness_metalic_c_doublesided.data[3], 1.0f, 0.0f, 1.0f);

					static const float TEX_CUBE_SIZE = 64.f;

					if (mMesh->m_materials[i].m_diffuseMapTex)
					{
						ImGui::Text("Diffuse Map");
						ImGui::SameLine();

						uint32_t width, height;
						mMesh->m_materials[i].m_diffuseMapTex->GetDimension(width, height);

						float scale = std::min(TEX_CUBE_SIZE / width, TEX_CUBE_SIZE / height);
						ImGui::ImageButton((ImTextureID)mMesh->m_materials[i].m_diffuseMapTex->GetTexture(), ImVec2(width * scale, height * scale));
					}
					if (mMesh->m_materials[i].m_normalMapTex)
					{
						ImGui::Text("Normal Map");
						ImGui::SameLine();

						uint32_t width, height;
						mMesh->m_materials[i].m_normalMapTex->GetDimension(width, height);

						float scale = std::min(TEX_CUBE_SIZE / width, TEX_CUBE_SIZE / height);
						ImGui::ImageButton((ImTextureID)mMesh->m_materials[i].m_normalMapTex->GetTexture(), ImVec2(width * scale, height * scale));
					}
					if (mMesh->m_materials[i].m_bumpMapTex)
					{
						ImGui::Text("Bump Map");
						ImGui::SameLine();

						uint32_t width, height;
						mMesh->m_materials[i].m_bumpMapTex->GetDimension(width, height);

						float scale = std::min(TEX_CUBE_SIZE / width, TEX_CUBE_SIZE / height);
						ImGui::ImageButton((ImTextureID)mMesh->m_materials[i].m_bumpMapTex->GetTexture(), ImVec2(width * scale, height * scale));
					}

					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}
	}
}

void MeshRenderer::PrepareGPUBuffer()
{
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredGeometryPassEffect.get();
	m_associatedDeferredEffect->m_associatedMeshes.push_back(this);

	std::vector<Vertex::StdMeshVertex> vertices(mMesh->m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < mMesh->m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = mMesh->m_positionBufferCPU[i];
		vertices[k].Normal = mMesh->m_normalBufferCPU[i];
		vertices[k].Tangent = mMesh->m_tangentBufferCPU[i];
		vertices[k].TexUV = mMesh->m_texUVBufferCPU[i];
	}

	m_vertexBufferGPU = Renderer::Instance()->CreateGPUBuffer(vertices.size() * sizeof(Vertex::StdMeshVertex), BufferUsage::IMMUTABLE, (void*)&vertices.front(), BufferType::VERTEX_BUFFER);
	m_indexBufferGPU = Renderer::Instance()->CreateGPUBuffer(mMesh->m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&mMesh->m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);

	for (auto &subMesh : mMesh->m_subMeshes)
	{
		subMesh.PrepareGPUBuffer();
	}
}

void MeshRenderer::PrepareShadowMapBuffer()
{
	m_shadowMapVertexBufferGPU= Renderer::Instance()->CreateGPUBuffer(mMesh->m_positionBufferCPU.size() * sizeof(Vertex::ShadowMapVertex), BufferUsage::IMMUTABLE, (void*)&mMesh->m_positionBufferCPU.front(), BufferType::VERTEX_BUFFER);
}

void MeshRenderer::GatherDrawCall() {
	DeferredGeometryPassEffect* effect = (dynamic_cast<DeferredGeometryPassEffect*>(m_associatedDeferredEffect));

	UINT stride = sizeof(Vertex::StdMeshVertex);
	UINT offset = 0;

	Vec4 resourceMask(0, 0, 0, 0);

	if (m_receiveShadow)
		resourceMask.z() = 1;

	{
		int startIndex = 0;
		for (auto &subMesh: mMesh->m_subMeshes)
		{
			if (mMesh->m_materials[subMesh.m_matIndex].m_bumpMapTex)
				effect = EffectsManager::Instance()->m_deferredGeometryTessPassEffect.get();

			ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
			DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

			perObjData->Mat = mMesh->m_materials[subMesh.m_matIndex].m_attributes;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix() * mParent->GetTransform()->GetWorldTransform());
			perObjData->World = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransform());
			Mat4 worldView = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * mParent->GetTransform()->GetWorldTransform();
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform() * mParent->GetTransform()->GetWorldTransform());
			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

			resourceMask.x() = 0;
			resourceMask.y() = 0;

			switch (Renderer::GetRenderBackend())
			{
			case RenderBackend::D3D11:
			{
				cmd.srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
				cmd.srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
				perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransformInversed().Transpose());
				perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

				if (mMesh->m_materials[subMesh.m_matIndex].m_diffuseMapTex) {
					resourceMask.x() = 1;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(mMesh->m_materials[subMesh.m_matIndex].m_diffuseMapTex->GetTexture()), 0);
				}
				if (mMesh->m_materials[subMesh.m_matIndex].m_normalMapTex) {
					resourceMask.y() = 1;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(mMesh->m_materials[subMesh.m_matIndex].m_normalMapTex->GetTexture()), 1);
				}

				if (mMesh->m_materials[subMesh.m_matIndex].m_bumpMapTex) {
					cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(mMesh->m_materials[subMesh.m_matIndex].m_bumpMapTex->GetTexture()), 2);
				}
				else
				{
					cmd.type = PrimitiveTopology::TRIANGLELIST;
				}

				cmd.offset = (void*)(startIndex);
				break;
			}
			case RenderBackend::OPENGL4:
			{
				ConstantBuffer cbuffer2 = Renderer::Instance()->CreateConstantBuffer(2, sizeof(DeferredGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
				DeferredGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2->GetBuffer();

				if (mMesh->m_materials[subMesh.m_matIndex].m_diffuseMapTex > 0)
				{
					resourceMask.x() = 1;
					textureData->DiffuseMap = reinterpret_cast<uint64_t>(mMesh->m_materials[subMesh.m_matIndex].m_diffuseMapTex->GetTexture());
				}
				if (mMesh->m_materials[subMesh.m_matIndex].m_normalMapTex > 0)
				{
					resourceMask.y() = 1;
					textureData->NormalMap = reinterpret_cast<uint64_t>(mMesh->m_materials[subMesh.m_matIndex].m_normalMapTex->GetTexture());
				}
				if (mMesh->m_materials[subMesh.m_matIndex].m_bumpMapTex > 0)
				{
					cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
					textureData->BumpMapTex = reinterpret_cast<uint64_t>(mMesh->m_materials[subMesh.m_matIndex].m_bumpMapTex->GetTexture());
				}
				else
				{
					cmd.type = PrimitiveTopology::TRIANGLELIST;
				}
				textureData->ShadowMapTex = LightManager::Instance()->m_shadowMap->GetDepthTexHandle();
				textureData->CubeMapTex = Renderer::Instance()->GetSkyBox()->m_cubeMapTex;

				cmd.offset = (void*)(startIndex * sizeof(unsigned int));

				cmd.cbuffers.push_back(std::move(cbuffer2));
				break;
			}
			}

			perObjData->DiffX_NormY_ShadZ = resourceMask;

			cmd.flags = CmdFlag::DRAW;
			cmd.drawType = DrawType::INDEXED;
			cmd.inputLayout = effect->GetInputLayout();
			cmd.framebuffer = Renderer::Instance()->GetGbuffer();
			cmd.vertexBuffer.push_back((void*)m_vertexBufferGPU->GetBuffer());
			cmd.indexBuffer = (void*)m_indexBufferGPU->GetBuffer();
			cmd.vertexStride.push_back(stride);
			cmd.vertexOffset.push_back(offset);
			cmd.effect = effect;
			cmd.elementCount = subMesh.m_indexBufferCPU.size();
			cmd.cbuffers.push_back(std::move(cbuffer0));
			cmd.cbuffers.push_back(std::move(cbuffer1));

			Renderer::Instance()->AddDeferredDrawCmd(cmd);

			startIndex += subMesh.m_indexBufferCPU.size();
		}
	}
}

void MeshRenderer::GatherShadowDrawCall() {
	
	if (!m_castShadow)
		return;
	
	ShadowMapEffect* effect = EffectsManager::Instance()->m_shadowMapEffect.get();
	UINT stride = sizeof(Vertex::ShadowMapVertex);
	UINT offset = 0;

	{
		Mat4 worldViewProj = LightManager::Instance()->m_shadowMap->GetViewProjMatrix() * mParent->GetTransform()->GetWorldTransform();

		ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(ShadowMapEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);
		ShadowMapEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (ShadowMapEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
		perObjData->gWorldViewProj = TRASNPOSE_API_CHOOSER(worldViewProj);

		DrawCmd cmd;
		cmd.flags = CmdFlag::DRAW;
		cmd.drawType = DrawType::INDEXED;
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.vertexBuffer.push_back((void*)m_shadowMapVertexBufferGPU->GetBuffer());
		cmd.indexBuffer = (void*)m_indexBufferGPU->GetBuffer();
		cmd.vertexStride.push_back(stride);
		cmd.vertexOffset.push_back(offset);
		cmd.inputLayout = effect->GetInputLayout();
		cmd.framebuffer = LightManager::Instance()->m_shadowMap->GetRenderTarget();
		cmd.offset = (void*)(0);
		cmd.effect = effect;
		cmd.elementCount = (int64_t)mMesh->m_indexBufferCPU.size();
		cmd.cbuffers.push_back(std::move(cbuffer0));

		Renderer::Instance()->AddDeferredDrawCmd(cmd);
	}
}

}