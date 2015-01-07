#include "SubMesh.h"
#include "D3DIncludes.h"
#include "D3D11Renderer.h"

SubMesh::SubMesh() : m_indexBufferGPU(0), m_bumpMapSRV(0), m_diffuseMapSRV(0), m_normalMapSRV(0) {}

void SubMesh::PrepareGPUBuffer() {
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
}