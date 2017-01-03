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
		m_toRootTransform[i] = m_animation->GetTransform(m_joints[i].m_name + "_$AssimpFbx$_Rotation") * m_jointPreRotationBufferCPU[i] * m_animation->GetTransform(m_joints[i].m_name) * (m_joints[i].m_parentIdx == -1? IDENTITY_MAT : m_toRootTransform[m_joints[i].m_parentIdx]);
		m_matrixPalette[i] = TRASNPOSE_API_CHOOSER(m_joints[i].m_inverseBindPosMat * m_toRootTransform[i]);
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
		vertices[k].JointWeights = XMFLOAT4(&m_jointWeightsBufferCPU[i][0]);
		vertices[k].JointIndices = XMUINT4(&m_jointIndicesBufferCPU[i][0]);
	}

	m_vertexBufferGPU = new GPUBuffer(vertices.size() * sizeof(Vertex::SkinnedMeshVertex), BufferUsage::IMMUTABLE, (void*)&vertices.front(), BufferType::VERTEX_BUFFER);
	m_indexBufferGPU = new GPUBuffer(m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);
}

void SkinnedMesh::PrepareShadowMapBuffer()
{
}

void SkinnedMesh::GatherDrawCall()
{
	DeferredSkinnedGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredSkinnedGeometryPassEffect.get();

	UINT stride = sizeof(Vertex::SkinnedMeshVertex);
	UINT offset = 0;

	XMFLOAT4 resourceMask(0, 0, 0, 0);

	if (m_receiveShadow)
		resourceMask.z = 1;

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < m_subMeshes.size(); iSubMesh++) {

			ConstantBuffer cbuffer0(1, sizeof(DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1(0, sizeof(DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
			DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1.GetBuffer();

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

			//glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, effect->m_matrixPaletteSB, 0, sizeof(XMMATRIX) * m_matrixPalette.size());
			//void* ssbo = glMapNamedBufferRange(
			//	effect->m_matrixPaletteSB,
			//	0,
			//	sizeof(XMMATRIX) * m_matrixPalette.size(),
			//	GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
			//);
			//memcpy(ssbo, &m_matrixPalette[0], sizeof(XMMATRIX) * m_matrixPalette.size());
			//glUnmapNamedBuffer(effect->m_matrixPaletteSB);

			resourceMask.x = 0;
			resourceMask.y = 0;

			switch (Renderer::GetRenderBackend())
			{
			case RenderBackend::D3D11:
			{
				cmd.srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
				cmd.srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
				perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform())));
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
				ConstantBuffer cbuffer2(2, sizeof(DeferredSkinnedGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
				DeferredSkinnedGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredSkinnedGeometryPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2.GetBuffer();

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

void SkinnedMesh::GatherShadowDrawCall()
{
	DeferredSkinnedGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredSkinnedGeometryPassEffect.get();

	UINT stride = sizeof(Vertex::SkinnedMeshVertex);
	UINT offset = 0;

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < m_subMeshes.size(); iSubMesh++) {

			ConstantBuffer cbuffer0(1, sizeof(DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1(0, sizeof(DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
			DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1.GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = (CameraManager::Instance()->GetActiveCamera()->GetPos());

			perObjData->Mat = m_materials[m_subMeshes[iSubMesh].m_matIndex].m_attributes;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix());
			perObjData->World = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()));
			XMMATRIX worldView = XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewMatrix();;
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetTransform()->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewProjMatrix());

			switch (Renderer::GetRenderBackend())
			{
			case RenderBackend::D3D11:
			{
				// TODO m_matrixPalette should not be in a cbuffer
				ConstantBuffer cbuffer2(13, sizeof(DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER), effect->m_matrixPaletteSB);
				DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER* matrixPaletteData = (DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER*)cbuffer2.GetBuffer();

				memcpy(matrixPaletteData, &m_matrixPalette[0], sizeof(XMMATRIX) * m_matrixPalette.size());

				cmd.cbuffers.push_back(std::move(cbuffer2));
				break;
			}
			case RenderBackend::OPENGL4:
			{
				cmd.offset = (void*)(startIndex * sizeof(unsigned int));

				ConstantBuffer cbuffer2(15, sizeof(DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER), effect->m_matrixPaletteSB);
				DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER* matrixPaletteData = (DeferredSkinnedGeometryPassEffect::MATRIX_PALETTE_BUFFER*)cbuffer2.GetBuffer();

				memcpy(matrixPaletteData, &m_matrixPalette[0], sizeof(XMMATRIX) * m_matrixPalette.size());

				cmd.cbuffers.push_back(std::move(cbuffer2));
				break;
			}
			}

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

}