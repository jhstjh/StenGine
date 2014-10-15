#include "MeshRenderer.h"
#include "D3D11Renderer.h"
#include "EffectsManager.h"
#include "ObjReader.h"

MeshRenderer::MeshRenderer():
m_indexBufferCPU(0),
m_stdMeshVertexBufferGPU(0)
{
	//ObjReader::Read(L"Model/ball.obj", this);
	CreateBoxPrimitive();
	PrepareGPUBuffer();

	m_material.ambient = XMFLOAT4(0.2, 0.2, 0.2, 1);
	m_material.diffuse = XMFLOAT4(1.0, 0.5, 0.3, 1);
	m_material.specular = XMFLOAT4(1.0, 1.0, 1.0, 1);
}

MeshRenderer::~MeshRenderer() {
	ReleaseCOM(m_indexBufferGPU);
	ReleaseCOM(m_stdMeshVertexBufferGPU);
}

void MeshRenderer::CreateBoxPrimitive() {
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

// 
// 
// 	v[0] =  Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
// 	v[1] =  Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
// 	v[2] =  Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
// 	v[3] =  Vertex(+1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
// 		    
// 	v[4] =  Vertex(-1.0f, -1.0f, +1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
// 	v[5] =  Vertex(+1.0f, -1.0f, +1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
// 	v[6] =  Vertex(+1.0f, +1.0f, +1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
// 	v[7] =  Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
// 													 
// 	v[8] =  Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
// 	v[9] =  Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
// 	v[10] = Vertex(+1.0f, +1.0f, +1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
// 	v[11] = Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
// 
// 	v[12] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
// 	v[13] = Vertex(+1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
// 	v[14] = Vertex(+1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
// 	v[15] = Vertex(-1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
// 
// 	v[16] = Vertex(-1.0f, -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
// 	v[17] = Vertex(-1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
// 	v[18] = Vertex(-1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
// 	v[19] = Vertex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
// 
// 	v[20] = Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
// 	v[21] = Vertex(+1.0f, +1.0f, -1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
// 	v[22] = Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
// 	v[23] = Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);


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
}

void MeshRenderer::PrepareGPUBuffer() {

	std::vector<Vertex::StdMeshVertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
		vertices[k].Normal = m_normalBufferCPU[i];
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

	m_associatedEffect = EffectsManager::Instance()->m_effects[0];
	m_associatedEffect->m_associatedMeshes.push_back(this);
}

void MeshRenderer::Draw() {
	ID3DX11EffectTechnique* tech = m_associatedEffect->GetActiveTech();

	UINT stride = sizeof(Vertex::StdMeshVertex);
	UINT offset = 0;
	D3D11Renderer::Instance()->GetD3DContext()->IASetVertexBuffers(0, 1, &m_stdMeshVertexBufferGPU, &stride, &offset);
	D3D11Renderer::Instance()->GetD3DContext()->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);
	((StdMeshEffect*)m_associatedEffect)->Mat->SetRawValue(&m_material, 0, sizeof(Material));

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, D3D11Renderer::Instance()->GetD3DContext());

		D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
	}
}