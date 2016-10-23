#include "Graphics/D3D11/D3D11Texture.h"

namespace StenGine
{

D3D11Texture::D3D11Texture(uint32_t width, uint32_t height, void* srv)
	: m_width(width)
	, m_height(height)
	, m_textureSRV(reinterpret_cast<ID3D11ShaderResourceView*>(srv))
{

}

}
