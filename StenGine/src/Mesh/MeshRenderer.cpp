#include "Mesh/MeshRenderer.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/Component.h"

#if PLATFORM_WIN32
#include "Resource/ResourceManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Utility/ObjReader.h"
#include "Graphics/Effect/ShadowMap.h"
#endif

#include "Graphics/Effect/Skybox.h"
#include "Math/MathHelper.h"

#pragma warning(disable: 4312) // 'type cast': conversion from 'GLuint' to 'void *' of greater size
#pragma warning(disable: 4267) // conversion from 'size_t' to 'UINT', possible loss of data

Mesh::Mesh(int type = 0):
#if GRAPHICS_D3D11
m_indexBufferCPU(0),
m_stdMeshVertexBufferGPU(0),
m_shadowMapVertexBufferGPU(0),
m_diffuseMapSRV(0),
m_normalMapSRV(0),
m_bumpMapSRV(0),
#endif
m_castShadow(true),
m_receiveShadow(true)
{
	//ObjReader::Read(L"Model/ball.obj", this);
	if (type == 0)
		CreateBoxPrimitive();
	else if (type == 1)
		CreatePlanePrimitive();
}

Mesh::~Mesh() {
#if GRAPHICS_D3D11
	ReleaseCOM(m_diffuseMapSRV);
	ReleaseCOM(m_normalMapSRV);
	ReleaseCOM(m_bumpMapSRV);
#endif

	SafeDelete(m_shadowMapVertexBufferGPU);
	SafeDelete(m_stdMeshVertexBufferGPU);
	SafeDelete(m_indexBufferGPU);
	SafeDelete(m_associatedEffect);
	SafeDelete(m_associatedDeferredEffect);
}

void Mesh::Prepare() {

	PrepareGPUBuffer();
	PrepareShadowMapBuffer();

	for (uint32_t i = 0; i < m_subMeshes.size(); i++) {
		m_subMeshes[i].PrepareGPUBuffer();
	}
}

void Mesh::PrepareGPUBuffer() 
{
#if PLATFORM_WIN32
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredGeometryPassEffect;
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
#if PLATFORM_WIN32
	DeferredGeometryPassEffect* effect = (dynamic_cast<DeferredGeometryPassEffect*>(m_associatedDeferredEffect));

	UINT stride = sizeof(Vertex::StdMeshVertex);
	UINT offset = 0;

#if GRAPHICS_D3D11
	if (m_subMeshes[0].m_bumpMapSRV)
#endif
#if GRAPHICS_OPENGL
	if (m_subMeshes[0].m_bumpMapTex)
#endif
		effect = EffectsManager::Instance()->m_deferredGeometryTessPassEffect;


	XMFLOAT4 resourceMask(0, 0, 0, 0);

	if (m_receiveShadow)
		resourceMask.z = 1;

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		int startIndex = 0;
		for (uint32_t iSubMesh = 0; iSubMesh < m_subMeshes.size(); iSubMesh++) {

			ConstantBuffer cbuffer0(1, sizeof(DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER), (void*)effect->m_perFrameCB);
			ConstantBuffer cbuffer1(0, sizeof(DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER), (void*)effect->m_perObjectCB);

			DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredGeometryPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
			DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredGeometryPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1.GetBuffer();

			DrawCmd cmd;

			perframeData->EyePosW = (CameraManager::Instance()->GetActiveCamera()->GetPos());

			perObjData->Mat = m_material;
			perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
			perObjData->World = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()));
			XMMATRIX worldView = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
			perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);
			XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);

			perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

			resourceMask.x = 0;
			resourceMask.y = 0;

#if GRAPHICS_D3D11
			cmd.srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
			cmd.srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
			perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform())));
			perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

			if (m_subMeshes[iSubMesh].m_diffuseMapSRV) {
				resourceMask.x = 1;
				cmd.srvs.AddSRV(m_subMeshes[iSubMesh].m_diffuseMapSRV, 0);
			}
			if (m_subMeshes[iSubMesh].m_normalMapSRV) {
				resourceMask.y = 1;
				cmd.srvs.AddSRV(m_subMeshes[iSubMesh].m_normalMapSRV, 1);
			}
			
			if (m_subMeshes[iSubMesh].m_bumpMapSRV) {
				cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
				cmd.srvs.AddSRV(m_subMeshes[iSubMesh].m_bumpMapSRV, 2);
			}
			else 
			{
				cmd.type = PrimitiveTopology::TRIANGLELIST;
			}

			cmd.offset = (void*)(startIndex);
#endif

#if GRAPHICS_OPENGL
			if (m_subMeshes[iSubMesh].m_diffuseMapTex > 0)
				resourceMask.x = 1;
			if (m_subMeshes[iSubMesh].m_normalMapTex > 0)
				resourceMask.y = 1;
			if (m_subMeshes[iSubMesh].m_bumpMapTex > 0)
			{
				cmd.type = PrimitiveTopology::CONTROL_POINT_3_PATCHLIST;
			}
			else
			{
				cmd.type = PrimitiveTopology::TRIANGLELIST;
			}

			perObjData->DiffuseMap = m_subMeshes[iSubMesh].m_diffuseMapTex;
			perObjData->NormalMap = m_subMeshes[iSubMesh].m_normalMapTex;
			perObjData->ShadowMapTex = LightManager::Instance()->m_shadowMap->GetDepthTexHandle();
			perObjData->CubeMapTex = Renderer::Instance()->GetSkyBox()->m_cubeMapTex;			
			perObjData->BumpMapTex = m_subMeshes[iSubMesh].m_bumpMapTex;

			cmd.offset = (void*)(startIndex * sizeof(unsigned int));

#endif

			perObjData->DiffX_NormY_ShadZ = resourceMask;

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

#elif  PLATFORM_ANDROID
	DeferredGeometryPassEffect* effect = dynamic_cast<DeferredGeometryPassEffect*>(m_associatedDeferredEffect);
	
	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {

		effect->m_perObjUniformBuffer.WorldViewProj = (ndk_helper::Mat4)CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix() * *m_parents[iP]->GetWorldTransform();
		effect->m_perObjUniformBuffer.World = *m_parents[iP]->GetWorldTransform();
		effect->m_perObjUniformBuffer.WorldView = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * *m_parents[iP]->GetWorldTransform();
		//effect->m_perObjUniformBuffer.ShadowTransform = LightManager::Instance()->m_shadowMap->GetShadowMapTransform() * *m_parents[iP]->GetWorldTransform();

		int startIndex = 0;
		for (int iSubMesh = 0; iSubMesh < m_subMeshes.size(); iSubMesh++) {

			XMFLOAT4 resourceMask(0, 0, 0, 0);

// 			if (m_subMeshes[iSubMesh].m_diffuseMapTex > 0)
// 				resourceMask.x = 1;
// 			if (m_subMeshes[iSubMesh].m_normalMapTex > 0)
// 				resourceMask.y = 1;

			effect->m_perObjUniformBuffer.DiffX_NormY_ShadZ = resourceMask;

			// TODO: this is a bad practice here
			// should separate this material related cbuffer from transform
			effect->UpdateConstantBuffer();
			effect->BindConstantBuffer();

// 			effect->DiffuseMap = m_subMeshes[iSubMesh].m_diffuseMapTex;
// 			effect->NormalMap = m_subMeshes[iSubMesh].m_normalMapTex;
// 			effect->BindShaderResource();

			glDrawElements(
				GL_TRIANGLES,      // mode
				m_subMeshes[iSubMesh].m_indexBufferCPU.size(),    // count
																  //m_indexBufferCPU.size(),
				GL_UNSIGNED_INT,   // type
				(void*)(startIndex * sizeof(unsigned int))           // element array buffer offset
				);
//			effect->UnBindShaderResource();
			startIndex += m_subMeshes[iSubMesh].m_indexBufferCPU.size();
			//break;
		}
//		effect->UnBindConstantBuffer();
	}
#endif
}

void Mesh::GatherShadowDrawCall() {
	ShadowMapEffect* effect = EffectsManager::Instance()->m_shadowMapEffect;
	UINT stride = sizeof(Vertex::ShadowMapVertex);
	UINT offset = 0;
	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {

		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix();

		ConstantBuffer cbuffer0(0, sizeof(ShadowMapEffect::PEROBJ_CONSTANT_BUFFER), (void*)effect->m_perObjectCB);
		ShadowMapEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (ShadowMapEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
		perObjData->gWorldViewProj = TRASNPOSE_API_CHOOSER(worldViewProj);

		DrawCmd cmd;

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
