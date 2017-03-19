#include "Mesh/SubMesh.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Abstraction/RendererBase.h"

#include "imgui.h"

namespace StenGine
{

SubMesh::SubMesh() 
	: m_indexBufferGPU(0)
{

}

SubMesh::~SubMesh()
{
	SafeDelete(m_indexBufferGPU);
}

void SubMesh::PrepareGPUBuffer() {
	m_indexBufferGPU = new GPUBuffer(m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);
}

void SubMesh::DrawMenu()
{
	if (ImGui::CollapsingHeader("Sub Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{

	}
}

}