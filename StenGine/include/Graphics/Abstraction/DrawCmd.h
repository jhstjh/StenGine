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
#include "Graphics/Abstraction/UAVBinding.h"
#include "Graphics/Abstraction/ContextState.h"

namespace StenGine
{

enum class PrimitiveTopology : uint32_t
{
	POINTLIST,
	LINELIST,
	TRIANGLELIST,
	CONTROL_POINT_3_PATCHLIST,
	CONTROL_POINT_4_PATCHLIST,
};

enum class DrawType
{
	NONE,
	INDEXED,
	ARRAY,
};

struct CmdFlag
{
static const uint32_t	DRAW		 = 0x0001;
static const uint32_t	CLEAR_COLOR  = 0x0002;
static const uint32_t	CLEAR_DEPTH  = 0x0004;
static const uint32_t	BIND_FB		 = 0x0008; // bind framebuffer
static const uint32_t	SET_VP		 = 0x0010; // set viewport
static const uint32_t   SET_RSSTATE  = 0x0020; // set rasterizer state
static const uint32_t   COMPUTE      = 0x0040; 
static const uint32_t   SET_BS		 = 0x0080; // set blend state
static const uint32_t   SET_DS		 = 0x0100; // set depth state
static const uint32_t   SET_CS		 = 0x0200; // set cull state
static const uint32_t   SET_SS		 = 0x0400; // set scissor state
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

	uint32_t			threadGroupX    = 0;
	uint32_t			threadGroupY    = 0;
	uint32_t			threadGroupZ    = 0;

	std::vector<ConstantBuffer> cbuffers;
	UAVBinding			uavs;

	BlendState			blendState;
	DepthState			depthState;
	CullState			cullState;
	ScissorState		scissorState;

#if GRAPHICS_D3D11
	D3D11SRVBinding srvs;
	ID3D11RasterizerState* rsState		= nullptr;
#endif

#if GRAPHICS_OPENGL
	
#endif

};

}
#endif