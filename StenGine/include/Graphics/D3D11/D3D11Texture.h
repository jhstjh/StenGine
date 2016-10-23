#pragma once
#include "Graphics/D3DIncludes.h"
#include "Graphics/Abstraction/Texture.h"

namespace StenGine
{

class D3D11Texture : public TextureImpl
{
public:
	D3D11Texture(uint32_t width, uint32_t height, void* srv);

	virtual void* GetTexture() { return m_textureSRV; }
	virtual void GetDimension(uint32_t &width, uint32_t &height) { width = m_width; height = m_height; }

private:
	ID3D11ShaderResourceView* m_textureSRV;
	uint32_t m_width;
	uint32_t m_height;
};

}
