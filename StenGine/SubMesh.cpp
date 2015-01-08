#include "SubMesh.h"
#include "D3DIncludes.h"
#ifdef GRAPHICS_D3D11
#include "D3D11Renderer.h"
#endif

SubMesh::SubMesh() : m_indexBufferGPU(0), 
#ifdef GRAPHICS_D3D11
m_bumpMapSRV(0), m_diffuseMapSRV(0), m_normalMapSRV(0) 
#else
m_bumpMapTex(0), m_diffuseMapTex(0), m_normalMapTex(0)
#endif
{}

void SubMesh::PrepareGPUBuffer() {
#ifdef GRAPHICS_D3D11
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
#endif
}