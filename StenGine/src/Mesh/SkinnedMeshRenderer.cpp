#include <imgui.h>
#include "Math/MathHelper.h"
#include "Mesh/SkinnedMesh.h"
#include "Mesh/SkinnedMeshRenderer.h"
#include "Engine/EventSystem.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Resource/ResourceManager.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/GameObject.h"
#include "Utility/ObjReader.h"

namespace StenGine
{

SkinnedMeshRenderer::SkinnedMeshRenderer() 
	: m_animation(nullptr)
{
	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::PRE_RENDER, [this]() {	PrepareMatrixPalette(); });
}

SkinnedMeshRenderer::~SkinnedMeshRenderer() 
{

}

void SkinnedMeshRenderer::SetMesh(Mesh* mesh)
{
	mSkinnedMesh = dynamic_cast<SkinnedMesh*>(mesh);
	assert(mSkinnedMesh != nullptr);
	Prepare();
}

void SkinnedMeshRenderer::PrepareMatrixPalette()
{
	m_matrixPalette.resize(mSkinnedMesh->m_joints.size());
	m_toRootTransform.resize(mSkinnedMesh->m_joints.size());

	for (int64_t i = mSkinnedMesh->m_joints.size() - 1; i >= 0; --i)
	{
		// m_toRootTransform[i] = m_animation->GetTransform(m_joints[i].m_name + "_$AssimpFbx$_Rotation") * m_jointPreRotationBufferCPU[i] * m_animation->GetTransform(m_joints[i].m_name) * (m_joints[i].m_parentIdx == -1? Mat4::Identity() : m_toRootTransform[m_joints[i].m_parentIdx]);
		m_toRootTransform[i] = (mSkinnedMesh->m_joints[i].m_parentIdx == -1 ? Mat4::Identity() : m_toRootTransform[mSkinnedMesh->m_joints[i].m_parentIdx]) * m_animation->GetTransform(mSkinnedMesh->m_joints[i].m_name) * mSkinnedMesh->m_jointPreRotationBufferCPU[i] * m_animation->GetTransform(mSkinnedMesh->m_joints[i].m_name + "_$AssimpFbx$_Rotation");
		// m_matrixPalette[i] = TRASNPOSE_API_CHOOSER(m_joints[i].m_inverseBindPosMat * m_toRootTransform[i]);
		m_matrixPalette[i] = TRASNPOSE_API_CHOOSER(m_toRootTransform[i] * mSkinnedMesh->m_joints[i].m_inverseBindPosMat);
	}
}

void SkinnedMeshRenderer::PrepareGPUBuffer()
{
	std::vector<Vertex::SkinnedMeshVertex> vertices(mSkinnedMesh->m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < mSkinnedMesh->m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = mSkinnedMesh->m_positionBufferCPU[i];
		vertices[k].Normal = mSkinnedMesh->m_normalBufferCPU[i];
		vertices[k].Tangent = mSkinnedMesh->m_tangentBufferCPU[i];
		vertices[k].TexUV = mSkinnedMesh->m_texUVBufferCPU[i];
		vertices[k].JointWeights = Vec4(&mSkinnedMesh->m_jointWeightsBufferCPU[i][0]);
		vertices[k].JointIndices = Vec4uiPacked(Vec4ui(&mSkinnedMesh->m_jointIndicesBufferCPU[i][0]));
	}

	m_vertexBufferGPU= Renderer::Instance()->CreateGPUBuffer(vertices.size() * sizeof(Vertex::SkinnedMeshVertex), BufferUsage::IMMUTABLE, (void*)&vertices.front(), BufferType::VERTEX_BUFFER);
	m_indexBufferGPU= Renderer::Instance()->CreateGPUBuffer(mSkinnedMesh->m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&mSkinnedMesh->m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);

	for (auto &subMesh : mSkinnedMesh->m_subMeshes)
	{
		subMesh.PrepareGPUBuffer();
	}
}

void SkinnedMeshRenderer::PrepareShadowMapBuffer()
{
}

void SkinnedMeshRenderer::GatherDrawCall()
{
	DeferredSkinnedGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredSkinnedGeometryPassEffect.get();

	UINT stride = sizeof(Vertex::SkinnedMeshVertex);
	UINT offset = 0;

	Vec4 resourceMask(0, 0, 0, 0);

	if (m_receiveShadow)
		resourceMask.z() = 1;

	{
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < mSkinnedMesh->m_subMeshes.size(); iSubMesh++) {

			ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
			DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

			perObjData->Mat = mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_attributes;
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
				perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransform().Inverse().Transpose());
				perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

				if (mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex) {
					resourceMask.x() = 1;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex->GetTexture()), 0);
				}
				if (mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex) {
					resourceMask.y() = 1;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex->GetTexture()), 1);
				}

				if (mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex) {
					cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
					cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_bumpMapTex->GetTexture()), 2);
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

				if (mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex > 0)
				{
					resourceMask.x() = 1;
					textureData->DiffuseMap = reinterpret_cast<uint64_t>(mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_diffuseMapTex->GetTexture());
				}
				if (mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex > 0)
				{
					resourceMask.y() = 1;
					textureData->NormalMap = reinterpret_cast<uint64_t>(mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_normalMapTex->GetTexture());
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
			cmd.vertexBuffer.push_back((void*)m_vertexBufferGPU->GetBuffer());
			cmd.indexBuffer = (void*)m_indexBufferGPU->GetBuffer();
			cmd.vertexStride.push_back(stride);
			cmd.vertexOffset.push_back(offset);
			cmd.effect = effect;
			cmd.elementCount = mSkinnedMesh->m_subMeshes[iSubMesh].m_indexBufferCPU.size();
			cmd.cbuffers.push_back(std::move(cbuffer0));
			cmd.cbuffers.push_back(std::move(cbuffer1));

			Renderer::Instance()->AddDeferredDrawCmd(cmd);

			startIndex += mSkinnedMesh->m_subMeshes[iSubMesh].m_indexBufferCPU.size();
		}
	}
}

void SkinnedMeshRenderer::GatherShadowDrawCall()
{
	DeferredSkinnedGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredSkinnedGeometryPassEffect.get();

	UINT stride = sizeof(Vertex::SkinnedMeshVertex);
	UINT offset = 0;

	{
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < mSkinnedMesh->m_subMeshes.size(); iSubMesh++) {

			ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
			ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

			DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredSkinnedGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
			DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredSkinnedGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

			perObjData->Mat = mSkinnedMesh->m_materials[mSkinnedMesh->m_subMeshes[iSubMesh].m_matIndex].m_attributes;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewProjMatrix() * mParent->GetTransform()->GetWorldTransform());
			perObjData->World = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransform());
			Mat4 worldView = LightManager::Instance()->m_shadowMap->GetViewMatrix() * mParent->GetTransform()->GetWorldTransform();
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform() * mParent->GetTransform()->GetWorldTransform());
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
			cmd.vertexBuffer.push_back((void*)m_vertexBufferGPU->GetBuffer());
			cmd.indexBuffer = (void*)m_indexBufferGPU->GetBuffer();
			cmd.vertexStride.push_back(stride);
			cmd.vertexOffset.push_back(offset);
			cmd.effect = effect;
			cmd.elementCount = mSkinnedMesh->m_subMeshes[iSubMesh].m_indexBufferCPU.size();
			cmd.cbuffers.push_back(std::move(cbuffer0));
			cmd.cbuffers.push_back(std::move(cbuffer1));

			Renderer::Instance()->AddDeferredDrawCmd(cmd);

			startIndex += mSkinnedMesh->m_subMeshes[iSubMesh].m_indexBufferCPU.size();
		}
	}
}

}