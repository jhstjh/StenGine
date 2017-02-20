#include <algorithm>
#include "Mesh/MeshRenderer.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/GameObject.h"

#include "Resource/ResourceManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Utility/ObjReader.h"
#include "Graphics/Effect/ShadowMap.h"

#include "Graphics/Effect/Skybox.h"
#include "Math/MathHelper.h"
#include "imgui.h"

#pragma warning(disable: 4312) // 'type cast': conversion from 'GLuint' to 'void *' of greater size
#pragma warning(disable: 4267) // conversion from 'size_t' to 'UINT', possible loss of data
#pragma warning(disable: 4996)

namespace StenGine
{

Mesh::Mesh(int type = 0) 
	: m_indexBufferCPU(0)
	, m_castShadow(true)
	, m_receiveShadow(true)
	, m_associatedEffect(nullptr)
	, m_associatedDeferredEffect(nullptr)
	, m_vertexBufferGPU(nullptr)
	, m_shadowMapVertexBufferGPU(nullptr)
{
	//ObjReader::Read(L"Model/ball.obj", this);
	if (type == 0)
		CreateBoxPrimitive();
	else if (type == 1)
		CreatePlanePrimitive();
}

Mesh::~Mesh() {
	SafeDelete(m_shadowMapVertexBufferGPU);
	SafeDelete(m_vertexBufferGPU);
	SafeDelete(m_indexBufferGPU);
}

void Mesh::Prepare() {

	PrepareGPUBuffer();
	PrepareShadowMapBuffer();

	for (uint32_t i = 0; i < m_subMeshes.size(); i++) {
		m_subMeshes[i].PrepareGPUBuffer();
	}
}

void Mesh::DrawMenu()
{
	if (ImGui::CollapsingHeader("Mesh Renderer", nullptr, true, true))
	{
		ImGui::Checkbox("Cast Shadow", &m_castShadow);
		ImGui::Checkbox("Receive Shadow", &m_receiveShadow);

		if (ImGui::TreeNode("Materials"))
		{
			char scratch[32];
			for (size_t i = 0; i < m_materials.size(); ++i)
			{
				sprintf(scratch, "Material%zd", i);
				if (ImGui::TreeNode(scratch))
				{
					ImGui::DragFloat3("Diffuse", reinterpret_cast<float*>(&m_materials[i].m_attributes.diffuse), 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat3("Roughness/Metalic/c/DoubleSided", reinterpret_cast<float*>(&m_materials[i].m_attributes.roughness_metalic_c_doublesided), 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("DoubleSided", &m_materials[i].m_attributes.roughness_metalic_c_doublesided.w, 1.0f, 0.0f, 1.0f);

					static const float TEX_CUBE_SIZE = 64.f;

					if (m_materials[i].m_diffuseMapTex)
					{
						ImGui::Text("Diffuse Map");
						ImGui::SameLine();

						uint32_t width, height;
						m_materials[i].m_diffuseMapTex->GetDimension(width, height);

						float scale = std::min(TEX_CUBE_SIZE / width, TEX_CUBE_SIZE / height);
						ImGui::ImageButton((ImTextureID)m_materials[i].m_diffuseMapTex->GetTexture(), ImVec2(width * scale, height * scale));
					}
					if (m_materials[i].m_normalMapTex)
					{
						ImGui::Text("Normal Map");
						ImGui::SameLine();

						uint32_t width, height;
						m_materials[i].m_normalMapTex->GetDimension(width, height);

						float scale = std::min(TEX_CUBE_SIZE / width, TEX_CUBE_SIZE / height);
						ImGui::ImageButton((ImTextureID)m_materials[i].m_normalMapTex->GetTexture(), ImVec2(width * scale, height * scale));
					}
					if (m_materials[i].m_bumpMapTex)
					{
						ImGui::Text("Bump Map");
						ImGui::SameLine();

						uint32_t width, height;
						m_materials[i].m_bumpMapTex->GetDimension(width, height);

						float scale = std::min(TEX_CUBE_SIZE / width, TEX_CUBE_SIZE / height);
						ImGui::ImageButton((ImTextureID)m_materials[i].m_bumpMapTex->GetTexture(), ImVec2(width * scale, height * scale));
					}

					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}
	}
}

void Mesh::PrepareGPUBuffer()
{
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredGeometryPassEffect.get();
	m_associatedDeferredEffect->m_associatedMeshes.push_back(this);

	std::vector<Vertex::StdMeshVertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
		vertices[k].Normal = m_normalBufferCPU[i];
		vertices[k].Tangent = m_tangentBufferCPU[i];
		vertices[k].TexUV = m_texUVBufferCPU[i];
	}

	m_vertexBufferGPU = new GPUBuffer(vertices.size() * sizeof(Vertex::StdMeshVertex), BufferUsage::IMMUTABLE, (void*)&vertices.front(), BufferType::VERTEX_BUFFER);
	m_indexBufferGPU = new GPUBuffer(m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);
}

void Mesh::PrepareShadowMapBuffer()
{
	m_shadowMapVertexBufferGPU = new GPUBuffer(m_positionBufferCPU.size() * sizeof(Vertex::ShadowMapVertex), BufferUsage::IMMUTABLE, (void*)&m_positionBufferCPU.front(), BufferType::VERTEX_BUFFER);
}

void Mesh::GatherDrawCall() {
	DeferredGeometryPassEffect* effect = (dynamic_cast<DeferredGeometryPassEffect*>(m_associatedDeferredEffect));

	UINT stride = sizeof(Vertex::StdMeshVertex);
	UINT offset = 0;

	XMFLOAT4 resourceMask(0, 0, 0, 0);

	if (m_receiveShadow)
		resourceMask.z = 1;

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < m_subMeshes.size(); iSubMesh++) {

			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex)
				effect = EffectsManager::Instance()->m_deferredGeometryTessPassEffect.get();

			ConstantBuffer cbuffer0(1, sizeof(DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1(0, sizeof(DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
			DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1.GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = XMFLOAT4( &CameraManager::Instance()->GetActiveCamera()->GetPos()[0] );

			perObjData->Mat = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_attributes;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix() * m_parents[iP]->GetTransform()->GetWorldTransform());
			perObjData->World = TRASNPOSE_API_CHOOSER(m_parents[iP]->GetTransform()->GetWorldTransform());
			Mat4 worldView = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * m_parents[iP]->GetTransform()->GetWorldTransform();
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();

			// TODO FIX ME
			auto shadowTrans = LightManager::Instance()->m_shadowMap->GetShadowMapTransform();
			XMFLOAT4X4 shadowTrans4x4;
			XMStoreFloat4x4(&shadowTrans4x4, shadowTrans);
			Mat4 shadowTransMat{ shadowTrans4x4.m[0] };

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(shadowTransMat * m_parents[iP]->GetTransform()->GetWorldTransform());
			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

			resourceMask.x = 0;
			resourceMask.y = 0;

			switch (Renderer::GetRenderBackend())
			{
			case RenderBackend::D3D11:
			{
				cmd.srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
				cmd.srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
				perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(m_parents[iP]->GetTransform()->GetWorldTransform().Inverse().Transpose());
				perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex) {
					resourceMask.x = 1;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex->GetTexture()), 0);
				}
				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex) {
					resourceMask.y = 1;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex->GetTexture()), 1);
				}

				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex) {
					cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex->GetTexture()), 2);
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
				ConstantBuffer cbuffer2(2, sizeof(DeferredGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
				DeferredGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2.GetBuffer();

				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex > 0)
				{
					resourceMask.x = 1;
					textureData->DiffuseMap = reinterpret_cast<uint64_t>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex->GetTexture());
				}
				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex > 0)
				{
					resourceMask.y = 1;
					textureData->NormalMap = reinterpret_cast<uint64_t>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex->GetTexture());
				}
				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex > 0)
				{
					cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
					textureData->BumpMapTex = reinterpret_cast<uint64_t>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex->GetTexture());
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
			cmd.framebuffer = &Renderer::Instance()->GetGbuffer();
			cmd.vertexBuffer = (void*)m_vertexBufferGPU->GetBuffer();
			cmd.indexBuffer = (void*)m_indexBufferGPU->GetBuffer();
			cmd.vertexStride = stride;
			cmd.vertexOffset = offset;
			cmd.effect = effect;
			cmd.elementCount = m_subMeshes[iSubMesh].m_indexBufferCPU.size();
			cmd.cbuffers.push_back(std::move(cbuffer0));
			cmd.cbuffers.push_back(std::move(cbuffer1));

			Renderer::Instance()->AddDeferredDrawCmd(cmd);

			startIndex += m_subMeshes[iSubMesh].m_indexBufferCPU.size();
		}
	}
}

void Mesh::GatherShadowDrawCall() {
	
	if (!m_castShadow)
		return;
	
	ShadowMapEffect* effect = EffectsManager::Instance()->m_shadowMapEffect.get();
	UINT stride = sizeof(Vertex::ShadowMapVertex);
	UINT offset = 0;

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {

		// TODO FIX ME
		auto shadowVPMat = LightManager::Instance()->m_shadowMap->GetViewProjMatrix();
		XMFLOAT4X4 shadowVPMat4x4;
		XMStoreFloat4x4(&shadowVPMat4x4, shadowVPMat);
		Mat4 shadowVPMatMat{ shadowVPMat4x4.m[0] };

		Mat4 worldViewProj = shadowVPMatMat * m_parents[iP]->GetTransform()->GetWorldTransform();

		ConstantBuffer cbuffer0(0, sizeof(ShadowMapEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);
		ShadowMapEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (ShadowMapEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
		perObjData->gWorldViewProj = TRASNPOSE_API_CHOOSER(worldViewProj);

		DrawCmd cmd;

		cmd.flags = CmdFlag::DRAW;
		cmd.drawType = DrawType::INDEXED;
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.vertexBuffer = (void*)m_shadowMapVertexBufferGPU->GetBuffer();
		cmd.indexBuffer = (void*)m_indexBufferGPU->GetBuffer();
		cmd.vertexStride = stride;
		cmd.vertexOffset = offset;
		cmd.inputLayout = effect->GetInputLayout();
		cmd.framebuffer = &LightManager::Instance()->m_shadowMap->GetRenderTarget();
		cmd.offset = (void*)(0);
		cmd.effect = effect;
		cmd.elementCount = (int64_t)m_indexBufferCPU.size();
		cmd.cbuffers.push_back(std::move(cbuffer0));

		Renderer::Instance()->AddDeferredDrawCmd(cmd);
	}
}

}