#include "GLTextureBinding.h"

GLTextureBinding::GLTextureBinding(GLint pos, uint64_t tex, uint32_t index, GLenum target)
	: m_pos(pos)
	, m_tex(tex)
	, m_index(index)
	, m_target(target)
{

}

void GLTextureBinding::Bind()
{
	//glActiveTexture(GL_TEXTURE0 + m_index);
	//glBindTexture(m_target, m_tex);
	//glUniform1i(m_pos, m_index);

	glUniformHandleui64ARB(m_pos, m_tex);
}
