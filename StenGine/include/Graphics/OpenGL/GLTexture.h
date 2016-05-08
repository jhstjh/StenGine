#pragma once
#include "glew.h"
#include <stdint.h>

namespace StenGine
{

class GLTexture
{
public:
	GLTexture(uint32_t width, uint32_t height, GLuint tex);
	~GLTexture();

	uint64_t GetTexture() { return m_textureHandle; }
	void GetDimension(uint32_t &width, uint32_t &height) { width = m_width; height = m_height; }

private:
	GLuint m_texture;
	uint64_t m_textureHandle;
	uint32_t m_width;
	uint32_t m_height;
};

using Texture = GLTexture;
}
