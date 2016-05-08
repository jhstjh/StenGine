#pragma once
#include "Graphics/D3DIncludes.h"

namespace StenGine
{

class D3D11Texture
{
public:
	D3D11Texture(uint32_t width, uint32_t height, ID3D11ShaderResourceView* srv);

	ID3D11ShaderResourceView* GetTexture() { return m_textureSRV; }
	void GetDimension(uint32_t &width, uint32_t &height) { width = m_width; height = m_height; }

private:
	ID3D11ShaderResourceView* m_textureSRV;
	uint32_t m_width;
	uint32_t m_height;
};

using Texture = D3D11Texture;
}
