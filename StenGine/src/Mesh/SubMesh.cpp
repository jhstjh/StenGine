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

}

void SubMesh::PrepareGPUBuffer() {
	m_indexBufferGPU = Renderer::Instance()->CreateGPUBuffer(m_indexBufferCPU.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&m_indexBufferCPU.front(), BufferType::INDEX_BUFFER);
}

void SubMesh::DrawMenu()
{
	if (ImGui::CollapsingHeader("Sub Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{

	}
}

}