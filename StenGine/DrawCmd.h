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
	// type : triangle/line/point...
	uint32_t m_type;
	void* m_framebuffer;
	void* m_vertexArrayObject;

	int64_t m_elementCount;
	void*	m_offset;

	class Effect* m_effect;

	std::vector<ConstantBuffer> m_cbuffers;

#ifdef GRAPHICS_D3D11
	D3D11SRVBinding m_srvs;

	ID3D11Buffer* m_indexBuffer;
	ID3D11Buffer* m_vertexBuffer;
	uint32_t m_vertexStride;
	uint32_t m_vertexOffset;
#endif
};

#endif