#ifndef __SUB_MESH__
#define __SUB_MESH__

#include "System/API/PlatformAPIDefs.h"

#include "Scene/Component.h"
#include "Graphics/Effect/Material.h"

#if (PLATFORM_WIN32) || (SG_TOOL)
#include "Graphics/D3DIncludes.h"

#if GRAPHICS_OPENGL
#include "glew.h"
#endif

#endif

namespace StenGine
{

class SubMesh : public Component {
public:
	SubMesh();
	//~SubMesh();
	void PrepareGPUBuffer();

	std::vector<UINT> m_indexBufferCPU;

#if GRAPHICS_D3D11
	ID3D11Buffer* m_indexBufferGPU;
#else
	GLuint m_indexBufferGPU;
#endif

	uint32_t m_matIndex;

	virtual void DrawMenu() override;
};

}
#endif