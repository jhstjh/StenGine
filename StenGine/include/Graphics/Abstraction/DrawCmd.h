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
#include "Graphics/Abstraction/Viewport.h"
#include "Graphics/Abstraction/RenderTarget.h"

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
	CONTROL_POINT_4_PATCHLIST = GL_PATCHES,
#endif
};

enum class DrawType
{
	NONE,
	INDEXED,
	ARRAY,
};

struct CmdFlag
{
static const uint32_t	DRAW		 = 0x01;
static const uint32_t	CLEAR_COLOR  = 0x02;
static const uint32_t	CLEAR_DEPTH  = 0x04;
static const uint32_t	BIND_FB		 = 0x08; // bind framebuffer
static const uint32_t	SET_VP		 = 0x10; // set viewport
static const uint32_t   SET_RSSTATE  = 0x20; // set rasterizer state
};

struct DrawCmd {
	uint32_t			flags			= 0;
	DrawType			drawType		= DrawType::NONE;
	PrimitiveTopology	type			= PrimitiveTopology::TRIANGLELIST;
	RenderTarget		framebuffer;
	void*				inputLayout		= nullptr;

	int64_t				elementCount	= 0;
	void*				offset			= 0;

	class Effect*		effect			= nullptr;

	void*				indexBuffer		= nullptr;
	void*				vertexBuffer	= nullptr;

	uint32_t			vertexStride	= 0;
	uint32_t			vertexOffset	= 0;

	Viewport			viewport;

	std::vector<ConstantBuffer> cbuffers;

#if GRAPHICS_D3D11
	D3D11SRVBinding srvs;
	ID3D11RasterizerState* rsState		= nullptr;
#endif
};

}
#endif