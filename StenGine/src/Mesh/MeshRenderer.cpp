#include "Mesh/MeshRenderer.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/GameObject.h"

#if PLATFORM_WIN32
#include "Resource/ResourceManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Utility/ObjReader.h"
#include "Graphics/Effect/ShadowMap.h"
#endif

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
#if GRAPHICS_D3D11
	, m_stdMeshVertexBufferGPU(nullptr)
	, m_shadowMapVertexBufferGPU(nullptr)
#endif
{
	//ObjReader::Read(L"Model/ball.obj", this);
	if (type == 0)
		CreateBoxPrimitive();
	else if (type == 1)
		CreatePlanePrimitive();
}

Mesh::~Mesh() {
	SafeDelete(m_shadowMapVertexBufferGPU);
	SafeDelete(m_stdMeshVertexBufferGPU);
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

#if GRAPHICS_OPENGL
					if (m_materials[i].m_diffuseMapTex)
					{
						ImGui::Text("Diffuse Map");
						ImGui::SameLine();

						// TODO need a Texture class to store meta data
						ImGui::ImageButton((ImTextureID)m_materials[i].m_diffuseMapTex, ImVec2(64, 64));
					}
					if (m_materials[i].m_normalMapTex)
					{
						ImGui::Text("Normal Map");
						ImGui::SameLine();

						// TODO need a Texture class to store meta data
						ImGui::ImageButton((ImTextureID)m_materials[i].m_normalMapTex, ImVec2(64, 64));
					}
					if (m_materials[i].m_bumpMapTex)
					{
						ImGui::Text("Bump Map");
						ImGui::SameLine();

						// TODO need a Texture class to store meta data
						ImGui::ImageButton((ImTextureID)m_materials[i].m_bumpMapTex, ImVec2(64, 64));
					}
#endif

#if GRAPHICS_D3D11
					if (m_materials[i].m_diffuseMapSRV)
					{
						ImGui::Text("Diffuse Map");
						ImGui::SameLine();

						// TODO need a Texture class to store meta data
						ImGui::ImageButton((ImTextureID)m_materials[i].m_diffuseMapSRV, ImVec2(64, 64));
					}
					if (m_materials[i].m_normalMapSRV)
					{
						ImGui::Text("Normal Map");
						ImGui::SameLine();

						// TODO need a Texture class to store meta data
						ImGui::ImageButton((ImTextureID)m_materials[i].m_normalMapSRV, ImVec2(64, 64));
					}
					if (m_materials[i].m_bumpMapSRV)
					{
						ImGui::Text("Bump Map");
						ImGui::SameLine();

						// TODO need a Texture class to store meta data
						ImGui::ImageButton((ImTextureID)m_materials[i].m_bumpMapSRV, ImVec2(64, 64));
					}
#endif
					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}
	}
}

void Mesh::PrepareGPUBuffer()
{
#if PLATFORM_WIN32
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredGeometryPassEffect.get();
	m_associatedDeferredEffect->m_associatedMeshes.push_back(this);
#elif  PLATFORM_ANDROID
	m_associatedEffect = EffectsManager::Instance()->m_simpleMeshEffect;
	m_associatedEffect->m_associatedMeshes.push_back(this);
#endif

	std::vector<Vertex::StdMeshVertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
		vertices[k].Normal = m_normalBufferCPU[i];
		vertices[k].Tangent = m_tangentBufferCPU[i];
		vertices[k].TexUV = m_texUVBufferCPU[i];
	}

	m_stdMeshVertexBufferGPU = new GPUBuffer(vertices.size() * sizeof(Vertex::StdMeshVertex), BufferUsage::IMMUTABLE, (void*)&vertices.front(), BufferType::VERTEX_BUFFER);
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

#if GRAPHICS_D3D11
			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapSRV)
#endif
#if GRAPHICS_OPENGL
			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex)
#endif
				effect = EffectsManager::Instance()->m_deferredGeometryTessPassEffect.get();


			ConstantBuffer cbuffer0(1, sizeof(DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), (void*)effect->m_perFrameCB);
			ConstantBuffer cbuffer1(0, sizeof(DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), (void*)effect->m_perObjectCB);

			DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
			DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1.GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = (CameraManager::Instance()->GetActiveCamera()->GetPos());

			perObjData->Mat = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_attributes;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
			perObjData->World = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()));
			XMMATRIX worldView = XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

			resourceMask.x = 0;
			resourceMask.y = 0;

#if GRAPHICS_D3D11
			cmd.srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
			cmd.srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
			perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform())));
			perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapSRV) {
				resourceMask.x = 1;
				cmd.srvs.AddSRV(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapSRV, 0);
			}
			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapSRV) {
				resourceMask.y = 1;
				cmd.srvs.AddSRV(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapSRV, 1);
			}

			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapSRV) {
				cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
				cmd.srvs.AddSRV(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapSRV, 2);
			}
			else
			{
				cmd.type = PrimitiveTopology::TRIANGLELIST;
			}

			cmd.offset = (void*)(startIndex);
#endif

#if GRAPHICS_OPENGL
			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex > 0)
				resourceMask.x = 1;
			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex > 0)
				resourceMask.y = 1;
			if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex > 0)
			{
				cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
			}
			else
			{
				cmd.type = PrimitiveTopology::TRIANGLELIST;
			}

			perObjData->DiffuseMap = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex;
			perObjData->NormalMap = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex;
			perObjData->ShadowMapTex = LightManager::Instance()->m_shadowMap->GetDepthTexHandle();
			perObjData->CubeMapTex = Renderer::Instance()->GetSkyBox()->m_cubeMapTex;
			perObjData->BumpMapTex = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex;

			cmd.offset = (void*)(startIndex * sizeof(unsigned int));

#endif

			perObjData->DiffX_NormY_ShadZ = resourceMask;

			cmd.flags = CmdFlag::DRAW;
			cmd.drawType = DrawType::INDEXED;
			cmd.inputLayout = effect->GetInputLayout();
			cmd.framebuffer = Renderer::Instance()->GetGbuffer();
			cmd.vertexBuffer = (void*)m_stdMeshVertexBufferGPU->GetBuffer();
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

		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix();

		ConstantBuffer cbuffer0(0, sizeof(ShadowMapEffect::PEROBJ_CONSTANT_BUFFER), (void*)effect->m_perObjectCB);
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
		cmd.framebuffer = LightManager::Instance()->m_shadowMap->GetRenderTarget();
		cmd.offset = (void*)(0);
		cmd.effect = effect;
		cmd.elementCount = (int64_t)m_indexBufferCPU.size();
		cmd.cbuffers.push_back(std::move(cbuffer0));

		Renderer::Instance()->AddShadowDrawCmd(cmd);
	}
}

}