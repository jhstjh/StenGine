#ifndef __GL_TEXTURE_BINDING_H_
#define __GL_TEXTURE_BINDING_H_

#include "GL/glew.h"
#include <stdint.h>

class GLTextureBinding
{
public:
	GLTextureBinding(GLint pos, GLint tex, uint32_t index, GLenum target);

	void Bind();

private:
	GLint m_pos;
	GLint m_tex;
	uint32_t m_index;
	GLenum m_target;
};

#endif