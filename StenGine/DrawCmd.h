#ifndef __DRAW_CMD_H_
#define __DRAW_CMD_H_

#include "GL/glew.h"
#include "EffectsManager.h"
#include <vector>
#include "GLTextureBinding.h"
#include "GLConstantBuffer.h"
// opengl version first

struct DrawCmd {
	// type : triangle/line/point...

	void* m_framebuffer;
	void* m_vertexArrayObject;

	int64_t m_elementCount;
	void*	m_offset;

	class Effect* m_effect;

	std::vector<GLConstantBuffer> m_cbuffers;
};

#endif