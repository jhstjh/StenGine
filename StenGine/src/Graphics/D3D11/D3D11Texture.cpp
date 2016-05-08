#include "Graphics/D3D11/D3D11Texture.h"

namespace StenGine
{

D3D11Texture::D3D11Texture(uint32_t width, uint32_t height, ID3D11ShaderResourceView * srv)
	: m_width(width)
	, m_height(height)
	, m_textureSRV(srv)
{

}

}
