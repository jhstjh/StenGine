#include "Mesh/SkinnedMesh.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/GameObject.h"

#include "Resource/ResourceManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Utility/ObjReader.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Engine/EventSystem.h"

#include "Graphics/Effect/Skybox.h"
#include "Math/MathHelper.h"
#include "imgui.h"

#include <functional>

namespace StenGine
{

SkinnedMesh::SkinnedMesh() 
	: Mesh(2) // TODO: just a temporary solution
	, m_animation(nullptr)
{
	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::PRE_RENDER, [this]() {	PrepareMatrixPalette(); });
}

SkinnedMesh::~SkinnedMesh() 
{

}

void SkinnedMesh::PrepareMatrixPalette()
{
	m_matrixPalette.resize(m_joints.size());
	m_toRootTransform.resize(m_joints.size());

	for (int64_t i = m_joints.size() - 1; i >= 0; --i)
	{
		// m_toRootTransform[i] = m_animation->GetTransform(m_joints[i].m_name + "_$AssimpFbx$_Rotation") * m_jointPreRotationBufferCPU[i] * m_animation->GetTransform(m_joints[i].m_name) * (m_joints[i].m_parentIdx == -1? Mat4::Identity() : m_toRootTransform[m_joints[i].m_parentIdx]);
		m_toRootTransform[i] = (m_joints[i].m_parentIdx == -1 ? Mat4::Identity() : m_toRootTransform[m_joints[i].m_parentIdx]) * m_animation->GetTransform(m_joints[i].m_name) * m_jointPreRotationBufferCPU[i] * m_animation->GetTransform(m_joints[i].m_name + "_$AssimpFbx$_Rotation");
		// m_matrixPalette[i] = TRASNPOSE_API_CHOOSER(m_joints[i].m_inverseBindPosMat * m_toRootTransform[i]);
		m_matrixPalette[i] = TRASNPOSE_API_CHOOSER(m_toRootTransform[i] * m_joints[i].m_inverseBindPosMat);
	}
}

void SkinnedMesh::PrepareGPUBuffer()
{
	std::vector<Vertex::SkinnedMeshVertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
		vertices[k].Normal = m_normalBufferCPU[i];
		vertices[k].Tangent = m_tangentBufferCPU[i];
		vertices[k].TexUV = m_texUVBufferCPU[i];
		vertices[k].JointWeights = Vec4(&m_jointWeightsBufferCPU[i][0]);
		vertices[k].JointIndices = Vec4uiPacked(Vec4ui(&m_jointIndicesBufferCPU[i][0]));
	}

	m_vertexBufferGPU= Renderer::Instance()->CreateGPUBuffer(vertices.size() * sizeof(Vertex::SkinnedMeshVertex), BufferUsage::IMMUTABLE, (void*)&vertices.front(), BufferType::VERTEX_BUFFER);
	m_indexBufferGPU= Renderer::Instance()->CreateGPUBuffer(m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);
}

void SkinnedMesh::PrepareShadowMapBuffer()
{
}

void SkinnedMesh::GatherDrawCall()
{
	DeferredSkinnedGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredSkinnedGeometryPassEffect.get();

	UINT stride = sizeof(Vertex::SkinnedMeshVertex);
	UINT offset = 0;

	Vec4 resourceMask(0, 0, 0, 0);

	if (m_receiveShadow)
		resourceMask.z() = 1;

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < m_subMeshes.size(); iSubMesh++) {

			ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
			DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

			perObjData->Mat = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_attributes;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix() * m_parents[iP]->GetTransform()->GetWorldTransform());
			perObjData->World = TRASNPOSE_API_CHOOSER(m_parents[iP]->GetTransform()->GetWorldTransform());
			Mat4 worldView = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * m_parents[iP]->GetTransform()->GetWorldTransform();
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform() * m_parents[iP]->GetTransform()->GetWorldTransform());
			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

			resourceMask.x() = 0;
			resourceMask.y() = 0;

			switch (Renderer::GetRenderBackend())
			{
			case RenderBackend::D3D11:
			{
				cmd.srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
				cmd.srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
				perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(m_parents[iP]->GetTransform()->GetWorldTransform().Inverse().Transpose());
				perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex) {
					resourceMask.x() = 1;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex->GetTexture()), 0);
				}
				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex) {
					resourceMask.y() = 1;
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
				ConstantBuffer cbuffer2 = Renderer::Instance()->CreateConstantBuffer(2, sizeof(DeferredSkinnedGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
				DeferredSkinnedGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredSkinnedGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2->GetBuffer();

				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex > 0)
				{
					resourceMask.x() = 1;
					textureData->DiffuseMap = reinterpret_cast<uint64_t>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex->GetTexture());
				}
				if (m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex > 0)
				{
					resourceMask.y() = 1;
					textureData->NormalMap = reinterpret_cast<uint64_t>(m_materials[m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex->GetTexture());
				}
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

void SkinnedMesh::GatherShadowDrawCall()
{
	DeferredSkinnedGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredSkinnedGeometryPassEffect.get();

	UINT stride = sizeof(Vertex::SkinnedMeshVertex);
	UINT offset = 0;

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < m_subMeshes.size(); iSubMesh++) {

			ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
			DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

			perObjData->Mat = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_attributes;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewProjMatrix() * m_parents[iP]->GetTransform()->GetWorldTransform());
			perObjData->World = TRASNPOSE_API_CHOOSER(m_parents[iP]->GetTransform()->GetWorldTransform());
			Mat4 worldView = LightManager::Instance()->m_shadowMap->GetViewMatrix() * m_parents[iP]->GetTransform()->GetWorldTransform();
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform() * m_parents[iP]->GetTransform()->GetWorldTransform());
			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewProjMatrix());

			switch (Renderer::GetRenderBackend())
			{
			case RenderBackend::D3D11:
			{
				// TODO m_matrixPalette should not be in a cbuffer
				ConstantBuffer cbuffer2 = Renderer::Instance()->CreateConstantBuffer(13, sizeof(DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER), effect->m_matrixPaletteSB);
				DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER* matrixPaletteData = (DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER*)cbuffer2->GetBuffer();

				memcpy(matrixPaletteData, &m_matrixPalette[0], sizeof(Mat4) * m_matrixPalette.size());

				cmd.cbuffers.push_back(std::move(cbuffer2));
				break;
			}
			case RenderBackend::OPENGL4:
			{
				cmd.offset = (void*)(startIndex * sizeof(unsigned int));

				ConstantBuffer cbuffer2 = Renderer::Instance()->CreateConstantBuffer(15, sizeof(DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER), effect->m_matrixPaletteSB);
				DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER* matrixPaletteData = (DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER*)cbuffer2->GetBuffer();

				memcpy(matrixPaletteData, &m_matrixPalette[0], sizeof(Mat4) * m_matrixPalette.size());

				cmd.cbuffers.push_back(std::move(cbuffer2));
				break;
			}
			}

			cmd.flags = CmdFlag::DRAW;
			cmd.drawType = DrawType::INDEXED;
			cmd.inputLayout = effect->GetInputLayout();
			cmd.framebuffer = Renderer::Instance()->GetGbuffer();
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

}