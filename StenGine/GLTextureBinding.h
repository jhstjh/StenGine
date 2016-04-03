#ifndef __GL_TEXTURE_BINDING_H_
#define __GL_TEXTURE_BINDING_H_

#include "GL/glew.h"
#include <stdint.h>

class GLTextureBinding
{
public:
	GLTextureBinding(GLint pos, uint64_t tex, uint32_t index, GLenum target);

	void Bind();

private:
	GLint m_pos;
	uint64_t m_tex;
	uint32_t m_index;
	GLenum m_target;
};

#endif