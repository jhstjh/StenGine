#include "MeshRenderer.h"
#include "D3D11Renderer.h"
#include "EffectsManager.h"
#include "ObjReader.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "SOIL.h"

Mesh::Mesh(int type = 0) :
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
#endif
}

void Mesh::Prepare() {
#ifdef GRAPHICS_OPENGL
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
#endif
	PrepareGPUBuffer();
	PrepareShadowMapBuffer();
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

	m_material.ambient = XMFLOAT4(0.2, 0.2, 0.2, 1);
	m_material.diffuse = XMFLOAT4(1.0, 0.5, 0.3, 1);
	m_material.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 10.0f);
#ifdef GRAPHICS_D3D11
// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		D3D11Renderer::Instance()->GetD3DDevice(),
// 		L"./Model/WoodCrate02.dds", 0, 0, &m_diffuseMapSRV, 0));
	CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/WoodCrate02.dds", nullptr, &m_diffuseMapSRV);
// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		D3D11Renderer::Instance()->GetD3DDevice(),
// 		L"./Model/WoodCrate02_normal.dds", 0, 0, &m_normalMapSRV, 0));
	CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/WoodCrate02_normal.dds", nullptr, &m_normalMapSRV);
#else
	// gl
	m_diffuseMap = SOIL_load_OGL_texture (
		"./Model/WoodCrate02.dds",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_TEXTURE_RECTANGLE
		);
	assert(m_diffuseMap != 0);

	m_normalMap = SOIL_load_OGL_texture(
		"./Model/WoodCrate02_normal.dds",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_TEXTURE_RECTANGLE
		);
	assert(m_normalMap != 0);
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

	m_material.ambient = XMFLOAT4(0.2, 0.2, 0.2, 1);
	m_material.diffuse = XMFLOAT4(1.0, 0.5, 0.3, 1);
	m_material.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

#ifdef GRAPHICS_D3D11
// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		D3D11Renderer::Instance()->GetD3DDevice(),
// 		L"./Model/darkbrickdxt1.dds", 0, 0, &m_diffuseMapSRV, 0));
	CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/darkbrickdxt1.dds", nullptr, &m_diffuseMapSRV);

// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		D3D11Renderer::Instance()->GetD3DDevice(),
// 		L"./Model/darkbrickdxt1_normal.dds", 0, 0, &m_normalMapSRV, 0));
	CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/darkbrickdxt1_normal.dds", nullptr, &m_normalMapSRV);

// 	HR(D3DX11CreateShaderResourceViewFromFile(
// 		D3D11Renderer::Instance()->GetD3DDevice(),
// 		L"./Model/darkbrickdxt1_bump.dds", 0, 0, &m_bumpMapSRV, 0));
	CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/darkbrickdxt1_bump.dds", nullptr, &m_bumpMapSRV);
#else
	// gl
#endif
}

void Mesh::PrepareGPUBuffer() {
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredGeometryPassEffect;
	m_associatedDeferredEffect->m_associatedMeshes.push_back(this);
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
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&vbd, &vinitData, &m_stdMeshVertexBufferGPU));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_indexBufferCPU.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &m_indexBufferCPU[0];
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&ibd, &iinitData, &m_indexBufferGPU));

	//m_associatedEffect = EffectsManager::Instance()->m_stdMeshEffect;
	//m_associatedEffect->m_associatedMeshes.push_back(this);
#else
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
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&vbd, &vinitData, &m_shadowMapVertexBufferGPU));
#else
	glGenVertexArrays(1, &m_shadowVertexArrayObject);
	glBindVertexArray(m_shadowVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBufferGPU);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
#endif
}

void Mesh::Draw() {
#ifdef GRAPHICS_D3D11
	UINT stride = sizeof(Vertex::StdMeshVertex);
	UINT offset = 0;

	D3D11Renderer::Instance()->GetD3DContext()->IASetVertexBuffers(0, 1, &m_stdMeshVertexBufferGPU, &stride, &offset);
	D3D11Renderer::Instance()->GetD3DContext()->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);

	DeferredGeometryPassEffect* deferredGeoEffect;

	if (!m_bumpMapSRV)
		deferredGeoEffect = (dynamic_cast<DeferredGeometryPassEffect*>(m_associatedDeferredEffect));
	else
		deferredGeoEffect = EffectsManager::Instance()->m_deferredGeometryTessPassEffect;

	deferredGeoEffect->SetShader();

	deferredGeoEffect->GetPerObjConstantBuffer()->Mat = m_material;
	XMFLOAT4 resourceMask(0, 0, 0, 0);
	if (m_diffuseMapSRV) {
		resourceMask.x = 1;
		//deferredGeoEffect->m_shaderResources[0] = m_diffuseMapSRV;
		deferredGeoEffect->SetShaderResources(m_diffuseMapSRV, 0);
	}
	if (m_normalMapSRV) {
		resourceMask.y = 1;
		//deferredGeoEffect->m_shaderResources[1] = m_normalMapSRV;
		deferredGeoEffect->SetShaderResources(m_normalMapSRV, 1);
	}
	if (m_receiveShadow)
		resourceMask.z = 1;
	deferredGeoEffect->GetPerObjConstantBuffer()->DiffX_NormY_ShadZ = resourceMask;

	//deferredGeoEffect->m_shaderResources[4] = D3D11Renderer::Instance()->m_SkyBox->m_cubeMapSRV;
	deferredGeoEffect->SetShaderResources(D3D11Renderer::Instance()->m_SkyBox->m_cubeMapSRV, 4);
	//deferredGeoEffect->m_shaderResources[3] = LightManager::Instance()->m_shadowMap->GetDepthSRV();
	deferredGeoEffect->SetShaderResources(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
	deferredGeoEffect->GetPerObjConstantBuffer()->ViewProj = XMMatrixTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	for (int iP = 0; iP < m_parents.size(); iP++) {

		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();
		deferredGeoEffect->GetPerObjConstantBuffer()->WorldViewProj = XMMatrixTranspose(worldViewProj);
		deferredGeoEffect->GetPerObjConstantBuffer()->World = XMMatrixTranspose(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()));
		XMMATRIX worldInvTranspose = MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()));
		deferredGeoEffect->GetPerObjConstantBuffer()->WorldInvTranspose = XMMatrixTranspose(worldInvTranspose);

		XMMATRIX worldView = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
		deferredGeoEffect->GetPerObjConstantBuffer()->WorldView = XMMatrixTranspose(worldView);

		XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);
		deferredGeoEffect->GetPerObjConstantBuffer()->WorldViewInvTranspose = XMMatrixTranspose(worldViewInvTranspose);

		XMMATRIX worldShadowMapTransform = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetShadowMapTransform();
		deferredGeoEffect->GetPerObjConstantBuffer()->ShadowTransform = XMMatrixTranspose(worldShadowMapTransform);

		if (m_bumpMapSRV) {

			D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			//deferredGeoEffect->m_shaderResources[3] = m_bumpMapSRV;
			DeferredGeometryTessPassEffect* realShader = dynamic_cast<DeferredGeometryTessPassEffect*>(deferredGeoEffect);
			//memcpy(&(realShader->m_perObjConstantBuffer), &(deferredGeoEffect->m_perObjConstantBuffer), sizeof(DeferredGeometryTessPassEffect::PEROBJ_CONSTANT_BUFFER));
			deferredGeoEffect->SetShaderResources(m_bumpMapSRV, 2);
			deferredGeoEffect->UpdateConstantBuffer();
			deferredGeoEffect->BindConstantBuffer();
			deferredGeoEffect->BindShaderResource();
			D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
			deferredGeoEffect->UnBindShaderResource();
			deferredGeoEffect->UnBindConstantBuffer();

			D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
		else {
			D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			deferredGeoEffect->UpdateConstantBuffer();
			deferredGeoEffect->BindConstantBuffer();
			deferredGeoEffect->BindShaderResource();
			D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
			deferredGeoEffect->UnBindShaderResource();
			deferredGeoEffect->UnBindConstantBuffer();
		}
	}
	deferredGeoEffect->UnSetShader();
#else
	glBindVertexArray(m_vertexArrayObject);
	
	
	DeferredGeometryPassEffect* effect = dynamic_cast<DeferredGeometryPassEffect*>(m_associatedDeferredEffect);
	
	effect->m_perObjUniformBuffer.Mat = m_material;

	effect->DiffuseMap = m_diffuseMap;
	effect->NormalMap = m_normalMap;

	for (int iP = 0; iP < m_parents.size(); iP++) {
		effect->m_perObjUniformBuffer.WorldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();
		effect->m_perObjUniformBuffer.World = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform());
		effect->m_perObjUniformBuffer.WorldView = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
		effect->UpdateConstantBuffer();
		effect->BindConstantBuffer();
		effect->BindShaderResource();

		glDrawElements(
			GL_TRIANGLES,      // mode
			m_indexBufferCPU.size(),    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
			);
		
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		effect->UnBindConstantBuffer();
		effect->UnBindShaderResource();
	}
#endif
}

void Mesh::DrawOnShadowMap() {
#ifdef GRAPHICS_D3D11
	UINT stride = sizeof(Vertex::ShadowMapVertex);
	UINT offset = 0;
	
	D3D11Renderer::Instance()->GetD3DContext()->IASetVertexBuffers(0, 1, &m_shadowMapVertexBufferGPU, &stride, &offset);
	D3D11Renderer::Instance()->GetD3DContext()->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);

	for (int iP = 0; iP < m_parents.size(); iP++) {
		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix();
		EffectsManager::Instance()->m_shadowMapEffect->m_perObjConstantBuffer.gWorldViewProj = XMMatrixTranspose(worldViewProj);
		EffectsManager::Instance()->m_shadowMapEffect->UpdateConstantBuffer();
		EffectsManager::Instance()->m_shadowMapEffect->BindConstantBuffer();
		D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
		EffectsManager::Instance()->m_shadowMapEffect->UnBindConstantBuffer();
	}
#else
	// gl render
#endif
}
