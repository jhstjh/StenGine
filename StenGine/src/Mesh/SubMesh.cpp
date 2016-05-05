#include "Mesh/SubMesh.h"
#if PLATFORM_WIN32
#include "Graphics/D3DIncludes.h"
#include "Graphics/Abstraction/RendererBase.h"
#endif

#include "imgui.h"

namespace StenGine
{

SubMesh::SubMesh() 
	: m_indexBufferGPU(0)
{

}

void SubMesh::PrepareGPUBuffer() {
#if GRAPHICS_D3D11
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
#endif
}

void SubMesh::DrawMenu()
{
	if (ImGui::CollapsingHeader("Sub Mesh", nullptr, true, true))
	{

	}
}

}