#include "MeshRenderer.h"
#include "D3D11Renderer.h"
#include "EffectsManager.h"
#include "ObjReader.h"
#include "CameraManager.h"
#include "LightManager.h"


Mesh::Mesh(int type = 0) :
m_indexBufferCPU(0),
m_stdMeshVertexBufferGPU(0),
m_shadowMapVertexBufferGPU(0),
m_diffuseMapSRV(0),
m_normalMapSRV(0),
m_bumpMapSRV(0),
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
	ReleaseCOM(m_indexBufferGPU);
	ReleaseCOM(m_stdMeshVertexBufferGPU);
	ReleaseCOM(m_shadowMapVertexBufferGPU);
	ReleaseCOM(m_diffuseMapSRV);
	ReleaseCOM(m_normalMapSRV);
	ReleaseCOM(m_bumpMapSRV);
}

void Mesh::Prepare() {
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

	HR(D3DX11CreateShaderResourceViewFromFile(
		D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/WoodCrate02.dds", 0, 0, &m_diffuseMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(
		D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/WoodCrate02_normal.dds", 0, 0, &m_normalMapSRV, 0));
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

	HR(D3DX11CreateShaderResourceViewFromFile(
		D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/darkbrickdxt1.dds", 0, 0, &m_diffuseMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(
		D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/darkbrickdxt1_normal.dds", 0, 0, &m_normalMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(
		D3D11Renderer::Instance()->GetD3DDevice(),
		L"./Model/darkbrickdxt1_bump.dds", 0, 0, &m_bumpMapSRV, 0));
}

void Mesh::PrepareGPUBuffer() {

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

	m_associatedEffect = EffectsManager::Instance()->m_stdMeshEffect;
	m_associatedEffect->m_associatedMeshes.push_back(this);
#if !FORWARD
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredShaderEffect;
	m_associatedDeferredEffect->m_associatedMeshes.push_back(this);
#endif
}

void Mesh::PrepareShadowMapBuffer() {

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
}

void Mesh::Draw() {
#if FORWARD
	ID3DX11EffectTechnique* tech = m_associatedEffect->GetActiveTech();
	// if it is std mesh
	if (dynamic_cast<StdMeshEffect*>(m_associatedEffect)) {
		UINT stride = sizeof(Vertex::StdMeshVertex);
		UINT offset = 0;
		D3D11Renderer::Instance()->GetD3DContext()->IASetVertexBuffers(0, 1, &m_stdMeshVertexBufferGPU, &stride, &offset);
		D3D11Renderer::Instance()->GetD3DContext()->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);

		(dynamic_cast<StdMeshEffect*>(m_associatedEffect))->Mat->SetRawValue(&m_material, 0, sizeof(Material));
		(dynamic_cast<StdMeshEffect*>(m_associatedEffect))->DiffuseMap->SetResource(m_diffuseMapSRV);


		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parent->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();
		(dynamic_cast<StdMeshEffect*>(m_associatedEffect))->WorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		(dynamic_cast<StdMeshEffect*>(m_associatedEffect))->World->SetMatrix(reinterpret_cast<float*>(m_parent->GetWorldTransform()));


		XMMATRIX world = XMLoadFloat4x4(m_parent->GetWorldTransform());
		XMMATRIX worldInvTranspose = InverseTranspose(world);
		(dynamic_cast<StdMeshEffect*>(m_associatedEffect))->WorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));


		XMMATRIX worldShadowMapTransform = XMLoadFloat4x4(m_parent->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetShadowMapTransform();
		(dynamic_cast<StdMeshEffect*>(m_associatedEffect))->ShadowTransform->SetMatrix(reinterpret_cast<float*>(&worldShadowMapTransform));


		D3DX11_TECHNIQUE_DESC techDesc;
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			tech->GetPassByIndex(p)->Apply(0, D3D11Renderer::Instance()->GetD3DContext());
			D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
		}
 	}
#else
	ID3DX11EffectTechnique* tech = m_associatedDeferredEffect->GetActiveTech();
		UINT stride = sizeof(Vertex::StdMeshVertex);
		UINT offset = 0;
		if (m_bumpMapSRV) {
			
		}
		else {
			
		}
		D3D11Renderer::Instance()->GetD3DContext()->IASetVertexBuffers(0, 1, &m_stdMeshVertexBufferGPU, &stride, &offset);
		D3D11Renderer::Instance()->GetD3DContext()->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);

		(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->Mat->SetRawValue(&m_material, 0, sizeof(Material));

		int resourceMask[3] = { 0 };
		if (m_diffuseMapSRV) {
			resourceMask[0] = 1;
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->DiffuseMap->SetResource(m_diffuseMapSRV);
		}
		if (m_normalMapSRV) {
			resourceMask[1] = 1;
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->NormalMap->SetResource(m_normalMapSRV);
		}
		if (m_receiveShadow)
			resourceMask[2] = 1;
		(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->DiffX_NormY_ShadZ->SetRawValue(resourceMask, 0, sizeof(int) * 3);
		
		(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->CubeMap->SetResource(D3D11Renderer::Instance()->m_SkyBox->m_cubeMapSRV);

		(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->ViewProj->SetMatrix(reinterpret_cast<float*>(&CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix()));
		
		for (int iP = 0; iP < m_parents.size(); iP++) {

			XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->WorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->World->SetMatrix(reinterpret_cast<float*>(m_parents[iP]->GetWorldTransform()));
			XMMATRIX worldInvTranspose = MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()));
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->WorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
			
			XMMATRIX worldView = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->WorldView->SetMatrix(reinterpret_cast<float*>(&worldView));

			XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->WorldViewInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldViewInvTranspose));

			XMMATRIX worldShadowMapTransform = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetShadowMapTransform();
			(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->ShadowTransform->SetMatrix(reinterpret_cast<float*>(&worldShadowMapTransform));


			if (m_bumpMapSRV) {
				D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
				(dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect))->BumpMap->SetResource(m_bumpMapSRV);
				tech = dynamic_cast<DeferredShaderEffect*>(m_associatedDeferredEffect)->DeferredShaderTessTech;
				D3DX11_TECHNIQUE_DESC techDesc;
				tech->GetDesc(&techDesc);
				for (UINT p = 0; p < techDesc.Passes; ++p)
				{
					tech->GetPassByIndex(p)->Apply(0, D3D11Renderer::Instance()->GetD3DContext());
					D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
				}
				D3D11Renderer::Instance()->GetD3DContext()->HSSetShader(0, 0, 0);
				D3D11Renderer::Instance()->GetD3DContext()->DSSetShader(0, 0, 0);
				D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}
			else {
				D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				D3DX11_TECHNIQUE_DESC techDesc;
				tech->GetDesc(&techDesc);
				for (UINT p = 0; p < techDesc.Passes; ++p)
				{
					tech->GetPassByIndex(p)->Apply(0, D3D11Renderer::Instance()->GetD3DContext());
					D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
				}
			}
		}

#endif
}

void Mesh::DrawOnShadowMap() {
	ID3DX11EffectTechnique* tech = EffectsManager::Instance()->m_shadowMapEffect->GetActiveTech();

	UINT stride = sizeof(Vertex::ShadowMapVertex);
	UINT offset = 0;
	D3D11Renderer::Instance()->GetD3DContext()->IASetVertexBuffers(0, 1, &m_shadowMapVertexBufferGPU, &stride, &offset);
	D3D11Renderer::Instance()->GetD3DContext()->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);

	for (int iP = 0; iP < m_parents.size(); iP++) {
		XMMATRIX worldViewProj = XMLoadFloat4x4(m_parents[iP]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix();
		EffectsManager::Instance()->m_shadowMapEffect->WorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

		D3DX11_TECHNIQUE_DESC techDesc;
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			tech->GetPassByIndex(p)->Apply(0, D3D11Renderer::Instance()->GetD3DContext());

			D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
		}
	}
}
