#include "MeshRenderer.h"
#include "EffectsManager.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "Component.h"

#ifdef PLATFORM_WIN32
#include "ResourceManager.h"
#include "RendererBase.h"
#include "ObjReader.h"
#include "ShadowMap.h"
#endif

#include "Skybox.h"
#include "MathHelper.h"

#pragma warning(disable: 4312) // 'type cast': conversion from 'GLuint' to 'void *' of greater size
#pragma warning(disable: 4267) // conversion from 'size_t' to 'UINT', possible loss of data

Mesh::Mesh(int type = 0):
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_indexBufferGPU);
	ReleaseCOM(m_stdMeshVertexBufferGPU);
	ReleaseCOM(m_shadowMapVertexBufferGPU);
	ReleaseCOM(m_diffuseMapSRV);
	ReleaseCOM(m_normalMapSRV);
	ReleaseCOM(m_bumpMapSRV);
	SafeDelete(m_associatedEffect);
	SafeDelete(m_associatedDeferredEffect);
#endif
}

void Mesh::Prepare() {

	PrepareGPUBuffer();
	PrepareShadowMapBuffer();

	for (uint32_t i = 0; i < m_subMeshes.size(); i++) {
		m_subMeshes[i].PrepareGPUBuffer();
	}
}

void Mesh::CreateBoxPrimitive() {
	m_positionBufferCPU.resize(24);
	m_positionBufferCPU = {
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

	m_normalBufferCPU.resize(24);
	m_normalBufferCPU = {
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
								   
		XMFLOAT3(0.0f, 0.0f, 1.0f ),
		XMFLOAT3(0.0f, 0.0f, 1.0f ),
		XMFLOAT3(0.0f, 0.0f, 1.0f ),
		XMFLOAT3(0.0f, 0.0f, 1.0f ),
								   
		XMFLOAT3(0.0f, 1.0f, 0.0f ),
		XMFLOAT3(0.0f, 1.0f, 0.0f ),
		XMFLOAT3(0.0f, 1.0f, 0.0f ),
		XMFLOAT3(0.0f, 1.0f, 0.0f ),
								   
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
								   
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
								   
		XMFLOAT3(1.0f, 0.0f, 0.0f ),
		XMFLOAT3(1.0f, 0.0f, 0.0f ),
		XMFLOAT3(1.0f, 0.0f, 0.0f ),
		XMFLOAT3(1.0f, 0.0f, 0.0f ),
	};

	m_tangentBufferCPU.resize(24);
	m_tangentBufferCPU = {
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),

		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),

		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),

		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),

		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),

		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
	};

	m_texUVBufferCPU.resize(24);
	m_texUVBufferCPU = {
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),
						   
		XMFLOAT2(1.0f, 1.0f),
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
						   
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),
						   
		XMFLOAT2(1.0f, 1.0f),
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
						   
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),
						   
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),
	};

	m_indexBufferCPU.resize(36);
	m_indexBufferCPU = {

		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

 		20, 21, 22,
 		20, 22, 23
	};

	m_subMeshes.resize(1);

	m_material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	m_material.diffuse = XMFLOAT4(1.0f, 0.5f, 0.3f, 1.f);
	m_material.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 10.0f);
	m_subMeshes[0].m_indexBufferCPU = m_indexBufferCPU;

#ifdef PLATFORM_WIN32
#ifdef GRAPHICS_D3D11
	m_subMeshes[0].m_diffuseMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(L"./Model/WoodCrate02.dds");
	m_subMeshes[0].m_normalMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(L"./Model/WoodCrate02_normal.dds");
#else
	m_subMeshes[0].m_diffuseMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(L"./Model/WoodCrate02.dds"));
	m_subMeshes[0].m_normalMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(L"./Model/WoodCrate02_normal.dds"));
#endif
#endif
}

void Mesh::CreatePlanePrimitive() {
	m_positionBufferCPU.resize(4);
	m_positionBufferCPU = {
		XMFLOAT3(-4.0f, -1.0f, -4.0f),
		XMFLOAT3(+4.0f, -1.0f, -4.0f),
		XMFLOAT3(+4.0f, -1.0f, +4.0f),
		XMFLOAT3(-4.0f, -1.0f, +4.0f),
	};

	m_normalBufferCPU.resize(4);
	m_normalBufferCPU = {
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
	};

	m_tangentBufferCPU.resize(4);
	m_tangentBufferCPU = {
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
	};

	m_texUVBufferCPU.resize(24);
	m_texUVBufferCPU = {
		XMFLOAT2(1.0f, 1.0f),
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
	};

	m_indexBufferCPU.resize(6);
	m_indexBufferCPU = {

		1, 0, 2,
		2, 0, 3,
	};

	m_material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	m_material.diffuse = XMFLOAT4(1.0f, 0.5f, 0.3f, 1.f);
	m_material.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

#ifdef GRAPHICS_D3D11
	m_diffuseMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(L"./Model/darkbrickdxt1.dds");
	m_normalMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(L"./Model/darkbrickdxt1_normal.dds");
	m_bumpMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(L"./Model/darkbrickdxt1_bump.dds");
#else
	// gl
#endif
}

void Mesh::PrepareGPUBuffer() {
#ifdef PLATFORM_WIN32
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredGeometryPassEffect;
	m_associatedDeferredEffect->m_associatedMeshes.push_back(this);
#elif defined PLATFORM_ANDROID
 	m_associatedEffect = EffectsManager::Instance()->m_simpleMeshEffect;
 	m_associatedEffect->m_associatedMeshes.push_back(this);
#endif

#ifdef GRAPHICS_D3D11
	std::vector<Vertex::StdMeshVertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
		vertices[k].Normal = m_normalBufferCPU[i];
		vertices[k].Tangent = m_tangentBufferCPU[i];
		vertices[k].TexUV = m_texUVBufferCPU[i];
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::StdMeshVertex) * m_positionBufferCPU.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	//vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&vbd, &vinitData, &m_stdMeshVertexBufferGPU));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_indexBufferCPU.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &m_indexBufferCPU[0];
	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&ibd, &iinitData, &m_indexBufferGPU));

	//m_associatedEffect = EffectsManager::Instance()->m_stdMeshEffect;
	//m_associatedEffect->m_associatedMeshes.push_back(this);
#elif defined(GRAPHICS_OPENGL) || defined(PLATFORM_ANDROID)
	glGenBuffers(1, &m_positionBufferGPU);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBufferGPU);
	glBufferData(GL_ARRAY_BUFFER, sizeof(XMFLOAT3) * m_positionBufferCPU.size(), &(m_positionBufferCPU[0]), GL_STATIC_DRAW);

	glGenBuffers(1, &m_normalBufferGPU);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferGPU);
	glBufferData(GL_ARRAY_BUFFER, sizeof(XMFLOAT3) * m_normalBufferCPU.size(), &(m_normalBufferCPU[0]), GL_STATIC_DRAW);

	glGenBuffers(1, &m_texUVBufferGPU);
	glBindBuffer(GL_ARRAY_BUFFER, m_texUVBufferGPU);
	glBufferData(GL_ARRAY_BUFFER, sizeof(XMFLOAT2) * m_texUVBufferCPU.size(), &(m_texUVBufferCPU[0]), GL_STATIC_DRAW);

	glGenBuffers(1, &m_tangentBufferGPU);
	glBindBuffer(GL_ARRAY_BUFFER, m_tangentBufferGPU);
	glBufferData(GL_ARRAY_BUFFER, sizeof(XMFLOAT3) * m_tangentBufferCPU.size(), &(m_tangentBufferCPU[0]), GL_STATIC_DRAW);

	glGenBuffers(1, &m_indexBufferGPU);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferGPU);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(UINT) * m_indexBufferCPU.size(), &(m_indexBufferCPU[0]), GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vertexArrayObject);
	glBindVertexArray(m_vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBufferGPU);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferGPU);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m_texUVBufferGPU);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m_tangentBufferGPU);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);
	glEnableVertexAttribArray (2);
	glEnableVertexAttribArray (3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferGPU);
#endif
}

void Mesh::PrepareShadowMapBuffer() {
#ifdef GRAPHICS_D3D11
	std::vector<Vertex::ShadowMapVertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::ShadowMapVertex) * m_positionBufferCPU.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	//vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&vbd, &vinitData, &m_shadowMapVertexBufferGPU));
#elif defined(GRAPHICS_OPENGL) || defined(PLATFORM_ANDROID)
	glGenVertexArrays(1, &m_shadowVertexArrayObject);
	glBindVertexArray(m_shadowVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBufferGPU);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferGPU);
#endif
}

void Mesh::GatherDrawCall() {
#ifdef PLATFORM_WIN32
	DeferredGeometryPassEffect* effect = (dynamic_cast<DeferredGeometryPassEffect*>(m_associatedDeferredEffect));

#ifdef GRAPHICS_D3D11
	UINT stride = sizeof(Vertex::StdMeshVertex);
	UINT offset = 0;

	if (m_subMeshes[0].m_bumpMapSRV)
		effect = EffectsManager::Instance()->m_deferredGeometryTessPassEffect;
#endif

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

			resourceMask.x = 0;
			resourceMask.y = 0;

#ifdef GRAPHICS_D3D11
			cmd.m_srvs.AddSRV(Renderer::Instance()->GetSkyBox()->m_cubeMapSRV, 4);
			cmd.m_srvs.AddSRV(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);

			perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
			perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform())));
			perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

			if (m_subMeshes[iSubMesh].m_diffuseMapSRV) {
				resourceMask.x = 1;
				cmd.m_srvs.AddSRV(m_subMeshes[iSubMesh].m_diffuseMapSRV, 0);
			}
			if (m_subMeshes[iSubMesh].m_normalMapSRV) {
				resourceMask.y = 1;
				cmd.m_srvs.AddSRV(m_subMeshes[iSubMesh].m_normalMapSRV, 1);
			}
			
			if (m_subMeshes[iSubMesh].m_bumpMapSRV) {
				cmd.m_type = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
				cmd.m_srvs.AddSRV(m_subMeshes[iSubMesh].m_bumpMapSRV, 2);
			}
			else 
			{
				cmd.m_type = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			}

			cmd.m_offset = (void*)(startIndex);
			cmd.m_vertexBuffer = m_stdMeshVertexBufferGPU;
			cmd.m_indexBuffer = m_indexBufferGPU;
			cmd.m_vertexStride = stride;
			cmd.m_vertexOffset = offset;
#endif

#ifdef GRAPHICS_OPENGL
			if (m_subMeshes[iSubMesh].m_diffuseMapTex > 0)
				resourceMask.x = 1;
			if (m_subMeshes[iSubMesh].m_normalMapTex > 0)
				resourceMask.y = 1;

			perObjData->DiffuseMap = m_subMeshes[iSubMesh].m_diffuseMapTex;
			perObjData->NormalMap = m_subMeshes[iSubMesh].m_normalMapTex;
			perObjData->ShadowMapTex = LightManager::Instance()->m_shadowMap->GetDepthTexHandle();
			perObjData->CubeMapTex = Renderer::Instance()->GetSkyBox()->m_cubeMapTex;

			cmd.m_type = GL_TRIANGLES;
			cmd.m_vertexArrayObject = (void*)m_vertexArrayObject;
			cmd.m_offset = (void*)(startIndex * sizeof(unsigned int));
#endif

			perObjData->DiffX_NormY_ShadZ = resourceMask;

			cmd.m_framebuffer = Renderer::Instance()->GetGbuffer();		
			cmd.m_effect = effect;
			cmd.m_elementCount = m_subMeshes[iSubMesh].m_indexBufferCPU.size();
			cmd.m_cbuffers.push_back(std::move(cbuffer0));
			cmd.m_cbuffers.push_back(std::move(cbuffer1));

			Renderer::Instance()->AddDeferredDrawCmd(cmd);
			
			startIndex += m_subMeshes[iSubMesh].m_indexBufferCPU.size();
		}
	}

#elif defined PLATFORM_ANDROID
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
#ifdef PLATFORM_WIN32
#ifdef GRAPHICS_D3D11
	UINT stride = sizeof(Vertex::ShadowMapVertex);
	UINT offset = 0;
	
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetVertexBuffers(0, 1, &m_shadowMapVertexBufferGPU, &stride, &offset);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);

	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix();
		EffectsManager::Instance()->m_shadowMapEffect->m_perObjConstantBuffer.gWorldViewProj = XMMatrixTranspose(worldViewProj);
		EffectsManager::Instance()->m_shadowMapEffect->UpdateConstantBuffer();
		EffectsManager::Instance()->m_shadowMapEffect->BindConstantBuffer();
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
		EffectsManager::Instance()->m_shadowMapEffect->UnBindConstantBuffer();
	}
#else
	for (uint32_t iP = 0; iP < m_parents.size(); iP++) {
		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix();
		EffectsManager::Instance()->m_shadowMapEffect->m_perObjUniformBuffer.gWorldViewProj = worldViewProj;

		ConstantBuffer cbuffer0(0, sizeof(ShadowMapEffect::PEROBJ_UNIFORM_BUFFER), (void*)effect->m_perObjectUBO);
		ShadowMapEffect::PEROBJ_UNIFORM_BUFFER* perObjData = (ShadowMapEffect::PEROBJ_UNIFORM_BUFFER*)cbuffer0.GetBuffer();
		perObjData->gWorldViewProj = worldViewProj;

		DrawCmd cmd;
		cmd.m_type = GL_TRIANGLES;
		cmd.m_framebuffer = LightManager::Instance()->m_shadowMap->GetRenderTarget();
		cmd.m_vertexArrayObject = (void*)m_vertexArrayObject;
		cmd.m_offset = (void*)(0);
		cmd.m_effect = effect;
		cmd.m_elementCount = (GLsizei)m_indexBufferCPU.size();
		cmd.m_cbuffers.push_back(std::move(cbuffer0));

		Renderer::Instance()->AddShadowDrawCmd(cmd);
	}
#endif
#endif
}
