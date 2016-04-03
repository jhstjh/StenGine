#ifndef __DRAW_CMD_H_
#define __DRAW_CMD_H_

#include "GL/glew.h"
#include "EffectsManager.h"
#include <vector>
#include "GLTextureBinding.h"
#include "GLConstantBuffer.h"
// opengl version first

struct DrawCmd {
	GLuint m_vertexArrayObject;

	GLsizei m_elementCount;
	void*	m_offset;

	class Effect* m_effect;

	std::vector<GLTextureBinding> m_textures;
	std::vector<GLConstantBuffer> m_cbuffers;
};

#endif