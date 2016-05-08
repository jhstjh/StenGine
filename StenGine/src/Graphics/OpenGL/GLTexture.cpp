#include "Graphics/OpenGL/GLTexture.h"

namespace StenGine
{

GLTexture::GLTexture(uint32_t width, uint32_t height, GLuint tex)
	: m_width(width)
	, m_height(height)
	, m_texture(tex)
{
	m_textureHandle = glGetTextureHandleARB(m_texture);
	glMakeTextureHandleResidentNV(m_textureHandle);
}

GLTexture::~GLTexture()
{
	glMakeTextureHandleNonResidentARB(m_textureHandle);
}

}
