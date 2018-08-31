#include "stdafx.h"

#include "Graphics/OpenGL/GLTexture.h"

namespace StenGine
{

GLTexture::GLTexture(uint32_t width, uint32_t height, void* tex)
	: m_width(width)
	, m_height(height)
	, m_texture(static_cast<GLuint>(reinterpret_cast<intptr_t>(tex)))
{
	m_textureHandle = glGetTextureHandleARB(m_texture);
	glMakeTextureHandleResidentNV(m_textureHandle);
}

GLTexture::~GLTexture()
{
	glMakeTextureHandleNonResidentARB(m_textureHandle);
	glDeleteTextures(1, &m_texture);
}

}
