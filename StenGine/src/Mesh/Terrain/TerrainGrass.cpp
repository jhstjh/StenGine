#include "stdafx.h"

#include <random>
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Math/MathHelper.h"
#include "Mesh/Mesh.h"
#include "Mesh/Terrain/Terrain.h"
#include "Mesh/Terrain/TerrainGrass.h"
#include "Resource/ResourceManager.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"

#if BUILD_DEBUG
#define DENSITY 100
#else
#define DENSITY 5000
#endif

namespace StenGine
{

TerrainGrass::TerrainGrass(Terrain* terrain, float width, float depth)
{
	mParent = terrain;
	mMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/grassPatch.fbx");

	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<float> disX(-width * 0.5f, width * 0.5f);
	std::uniform_real_distribution<float> disZ(-depth * 0.5f, depth * 0.5f);

	for (uint32_t i = 0; i < DENSITY; i++ )
	{
		Vertex::InstanceVertex instance;
		float x = disX(gen);
		float z = disZ(gen);
		instance.position = { x, mParent->GetHeight(x, z), z };
		mInstances.push_back(instance);
	}

	PrepareGPUBuffer();
}

void TerrainGrass::PrepareGPUBuffer()
{
	std::vector<Vertex::StdMeshVertex> vertices(mMesh->m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < mMesh->m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = mMesh->m_positionBufferCPU[i];
		vertices[k].Normal = mMesh->m_normalBufferCPU[i];
		vertices[k].Tangent = mMesh->m_tangentBufferCPU[i];
		vertices[k].TexUV = mMesh->m_texUVBufferCPU[i];
	}

	mVertexBufferGPU = Renderer::Instance()->CreateGPUBuffer(vertices.size() * sizeof(Vertex::StdMeshVertex), BufferUsage::IMMUTABLE, (void*)&vertices.front(), BufferType::VERTEX_BUFFER);
	mIndexBufferGPU = Renderer::Instance()->CreateGPUBuffer(mMesh->m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&mMesh->m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);
	mInstanceBuffer = Renderer::Instance()->CreateGPUBuffer(mInstances.size() * sizeof(Vertex::InstanceVertex), BufferUsage::IMMUTABLE, (void*)&mInstances.front(), BufferType::VERTEX_BUFFER);

	for (auto &subMesh : mMesh->m_subMeshes)
	{
		subMesh.PrepareGPUBuffer();
	}
}

void TerrainGrass::GatherDrawCall()
{
	if (!mMesh)
	{
		return;
	}

	DrawCmd disableCullCmd;
	disableCullCmd.flags = CmdFlag::SET_CS;
	RasterizerState &disableCullState = disableCullCmd.rasterizerState;
	disableCullState.cullFaceEnabled = false;
	Renderer::Instance()->AddDeferredDrawCmd(disableCullCmd);

	DeferredGeometryInstancedPassEffect* effect = EffectsManager::Instance()->m_deferredGeometryInstancedPassEffect.get();

	UINT stride = sizeof(Vertex::StdMeshVertex);
	UINT offset = 0;

	Vec4 resourceMask(0, 0, 0, 0);
	resourceMask.z() = 1;

	resourceMask.w() = 1; // clip alpha

	int startIndex = 0;
	assert(mMesh->m_subMeshes.size() == 1); // TODO support instancing with multiple submesh
	auto subMesh = mMesh->m_subMeshes[0];

	ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredGeometryInstancedPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
	ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredGeometryInstancedPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

	DeferredGeometryInstancedPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredGeometryInstancedPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
	DeferredGeometryInstancedPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredGeometryInstancedPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

	DrawCmd cmd;

	perframeData->EyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

	perObjData->Mat = mMesh->m_materials[subMesh.m_matIndex].m_attributes;
	perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix() * mParent->GetWorldTransform());
	perObjData->World = TRASNPOSE_API_CHOOSER(mParent->GetWorldTransform());
	Mat4 worldView = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * mParent->GetWorldTransform();
	perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
	Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();

	perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform() * mParent->GetWorldTransform());
	perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	resourceMask.x() = 0;
	resourceMask.y() = 0;

	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		cmd.srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
		cmd.srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
		perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(mParent->GetWorldTransform().Inverse().Transpose());
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
		ConstantBuffer cbuffer2 = Renderer::Instance()->CreateConstantBuffer(2, sizeof(DeferredGeometryInstancedPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
		DeferredGeometryInstancedPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredGeometryInstancedPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2->GetBuffer();

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

	cmd.flags = CmdFlag::DRAW | CmdFlag::BIND_FB;
	cmd.drawType = DrawType::INDEXED;
	cmd.inputLayout = effect->GetInputLayout();
	cmd.framebuffer = Renderer::Instance()->GetGbuffer();
	cmd.indexBuffer = (void*)mIndexBufferGPU->GetBuffer();
	cmd.vertexBuffer.push_back((void*)mVertexBufferGPU->GetBuffer());
	cmd.vertexStride.push_back(stride);
	cmd.vertexOffset.push_back(offset);
	cmd.vertexBuffer.push_back((void*)mInstanceBuffer->GetBuffer());
	cmd.vertexStride.push_back(sizeof(Vertex::InstanceVertex));
	cmd.vertexOffset.push_back(offset);
	cmd.instanceCount = static_cast<uint32_t>(mInstances.size());
	cmd.effect = effect;
	cmd.elementCount = subMesh.m_indexBufferCPU.size();
	cmd.cbuffers.push_back(std::move(cbuffer0));
	cmd.cbuffers.push_back(std::move(cbuffer1));

	Renderer::Instance()->AddDeferredDrawCmd(cmd);

	startIndex += static_cast<int32_t>(subMesh.m_indexBufferCPU.size());


	DrawCmd enableCullCmd;
	enableCullCmd.flags = CmdFlag::SET_CS;
	enableCullCmd.rsState = 0;

	Renderer::Instance()->AddDeferredDrawCmd(enableCullCmd);
}

}