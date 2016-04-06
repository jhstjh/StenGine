#ifndef __DRAW_CMD_H_
#define __DRAW_CMD_H_

#ifdef GRAPHICS_OPENGL
#include "GL/glew.h"
#endif

#ifdef GRAPHICS_D3D11
#include "D3D11SRVBinding.h"
#endif

#include "EffectsManager.h"
#include <vector>
#include "GLTextureBinding.h"
#include "ConstantBuffer.h"
// opengl version first

struct DrawCmd {
	uint32_t type;
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

#ifdef GRAPHICS_D3D11
	D3D11SRVBinding srvs;
#endif
};

#endif