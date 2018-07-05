#ifndef __SUB_MESH__
#define __SUB_MESH__

#include "System/API/PlatformAPIDefs.h"

#include "Scene/Component.h"
#include "Graphics/Effect/Material.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Abstraction/GPUBuffer.h"
#include "glew.h"

namespace StenGine
{

class SubMesh : public Component {
public:
	SubMesh();
	virtual ~SubMesh();
	void PrepareGPUBuffer();

	std::vector<UINT> m_indexBufferCPU;

	GPUBuffer m_indexBufferGPU;

	uint32_t m_matIndex;

	virtual void DrawMenu() override;
};

}
#endif