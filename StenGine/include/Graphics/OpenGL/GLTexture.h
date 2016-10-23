#pragma once
#include "glew.h"
#include <stdint.h>
#include "Graphics/Abstraction/Texture.h"

namespace StenGine
{

class GLTexture : public TextureImpl
{
public:
	GLTexture(uint32_t width, uint32_t height, void* tex);
	virtual ~GLTexture();

	virtual void* GetTexture() { return reinterpret_cast<void*>(m_textureHandle); }
	virtual void GetDimension(uint32_t &width, uint32_t &height) { width = m_width; height = m_height; }

private:
	GLuint m_texture;
	uint64_t m_textureHandle;
	uint32_t m_width;
	uint32_t m_height;
};

}
