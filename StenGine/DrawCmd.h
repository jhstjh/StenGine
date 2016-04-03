#ifndef __DRAW_CMD_H_
#define __DRAW_CMD_H_

#include "GL/glew.h"
#include "EffectsManager.h"
#include <vector>
#include "GLTextureBinding.h"
#include "GLConstantBuffer.h"
// opengl version first

struct DrawCmd {
	GLuint m_indexBufferGPU;
	GLuint m_indexBufferShadowGPU;

	GLuint m_positionBufferGPU;
	GLuint m_normalBufferGPU;
	GLuint m_texUVBufferGPU;
	GLuint m_tangentBufferGPU;

	GLuint m_vertexArrayObject;
	GLuint m_shadowVertexArrayObject;

	GLsizei m_elementCount;
	void*	m_offset;

	class Effect* m_effect;

	std::vector<GLTextureBinding> m_textures;
	std::vector<GLConstantBuffer> m_cbuffers;
};

#endif