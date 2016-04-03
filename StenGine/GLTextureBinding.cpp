#include "GLTextureBinding.h"

GLTextureBinding::GLTextureBinding(GLint pos, GLint tex, uint32_t index)
	: m_pos(pos)
	, m_tex(tex)
	, m_index(index)
{

}

void GLTextureBinding::Bind()
{
	glActiveTexture(GL_TEXTURE0 + m_index);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glUniform1i(m_pos, m_index);
}
