#ifndef __DRAW_CMD_H_
#define __DRAW_CMD_H_

#include <vector>

#if GRAPHICS_OPENGL
#include "glew.h"
#endif

#if GRAPHICS_D3D11
#include "Graphics/D3D11/D3D11SRVBinding.h"
#endif

#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Abstraction/ConstantBuffer.h"

namespace StenGine
{

enum class PrimitiveTopology : uint32_t
{
#if GRAPHICS_D3D11
	POINTLIST = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
	LINELIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
	TRIANGLELIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	CONTROL_POINT_3_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
	CONTROL_POINT_4_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
#elif  GRAPHICS_OPENGL
	POINTLIST = GL_POINTS,
	LINELIST = GL_LINES,
	TRIANGLELIST = GL_TRIANGLES,
	CONTROL_POINT_3_PATCHLIST = GL_PATCHES,
#endif
};

struct DrawCmd {
	PrimitiveTopology type;
	void* framebuffer;
	void* inputLayout;

	int64_t elementCount;
	void*	offset;

	class Effect* effect;

	void* indexBuffer;
	void* vertexBuffer;

	uint32_t vertexStride;
	uint32_t vertexOffset;

	std::vector<ConstantBuffer> cbuffers;

#if GRAPHICS_D3D11
	D3D11SRVBinding srvs;
#endif
};

}
#endif