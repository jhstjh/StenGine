#include "Mesh/SkinnedMesh.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3DIncludes.h"

SkinnedMesh::SkinnedMesh() 
	: Mesh(2) // TODO: just a temporary solution
{

}

SkinnedMesh::~SkinnedMesh() {

}

void SkinnedMesh::PrepareGPUBuffer() {
	m_associatedDeferredEffect = EffectsManager::Instance()->m_deferredGeometryPassEffect;
	m_associatedDeferredEffect->m_associatedMeshes.push_back(this);
#if GRAPHICS_D3D11
	std::vector<Vertex::SkinnedMeshVertex> vertices(m_positionBufferCPU.size());
	UINT k = 0;
	for (size_t i = 0; i < m_positionBufferCPU.size(); ++i, ++k)
	{
		vertices[k].Pos = m_positionBufferCPU[i];
		vertices[k].Normal = m_normalBufferCPU[i];
		vertices[k].Tangent = m_tangentBufferCPU[i];
		vertices[k].TexUV = m_texUVBufferCPU[i];
		vertices[k].JointWeights = m_jointWeightsBufferCPU[i];
		vertices[k].JointIndices = m_jointIndicesBufferCPU[i];
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::SkinnedMeshVertex) * m_positionBufferCPU.size();
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
#else
	// TODO: convert to skinned mesh
	glGenVertexArrays(1, &m_vertexArrayObject);
	glBindVertexArray(m_vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBufferGPU);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferGPU);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m_texUVBufferGPU);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m_tangentBufferGPU);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferGPU);
#endif
}

void SkinnedMesh::PrepareShadowMapBuffer() {

}