#include "MeshRenderer.h"
#include "D3D11Renderer.h"
#include "EffectsManager.h"

MeshRenderer::MeshRenderer():
m_colorBufferGPU(0),
m_indexBufferCPU(0),
m_positionBufferGPU(0),
m_Pos_Color_VertexBufferGPU(0)
{
	CreateBoxPrimitive();
	PrepareGPUBuffer();
}

MeshRenderer::~MeshRenderer() {
	ReleaseCOM(m_indexBufferGPU);
	ReleaseCOM(m_Pos_Color_VertexBufferGPU);
	ReleaseCOM(m_colorBufferGPU);
	ReleaseCOM(m_positionBufferGPU);
}

void MeshRenderer::CreateBoxPrimitive() {
	m_positionBufferCPU.resize(8);
	m_positionBufferCPU = {
		XMFLOAT3(-1.0f, -1.0f, -1.0f),
		XMFLOAT3(-1.0f, +1.0f, -1.0f),
		XMFLOAT3(+1.0f, +1.0f, -1.0f),
		XMFLOAT3(+1.0f, -1.0f, -1.0f),
		XMFLOAT3(-1.0f, -1.0f, +1.0f),
		XMFLOAT3(-1.0f, +1.0f, +1.0f),
		XMFLOAT3(+1.0f, +1.0f, +1.0f),
		XMFLOAT3(+1.0f, -1.0f, +1.0f),
	};

	m_colorBufferCPU.reserve(8);
	m_colorBufferCPU = {
		(const float*)&Colors::White,
		(const float*)&Colors::Black,
		(const float*)&Colors::Red,
		(const float*)&Colors::Green,
		(const float*)&Colors::Blue,
		(const float*)&Colors::Yellow,
		(const float*)&Colors::Cyan,
		(const float*)&Colors::Magenta,
	};

	m_indexBufferCPU.resize(36);
	m_indexBufferCPU = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};
}

void MeshRenderer::PrepareGPUBuffer() {

	std::vector<Vertex::Pos_Color_Vertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
		vertices[k].Color = m_colorBufferCPU[i];
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Pos_Color_Vertex) * m_positionBufferCPU.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	//vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&vbd, &vinitData, &m_Pos_Color_VertexBufferGPU));

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

	UINT stride = sizeof(Vertex::Pos_Color_Vertex);
	UINT offset = 0;
	D3D11Renderer::Instance()->GetD3DContext()->IASetVertexBuffers(0, 1, &m_Pos_Color_VertexBufferGPU, &stride, &offset);
	D3D11Renderer::Instance()->GetD3DContext()->IASetIndexBuffer(m_indexBufferGPU, DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, D3D11Renderer::Instance()->GetD3DContext());

		// 36 indices for the box.
		D3D11Renderer::Instance()->GetD3DContext()->DrawIndexed(m_indexBufferCPU.size(), 0, 0);
	}
}