#pragma once

#include "Graphics/D3DIncludes.h"
#include <array>

namespace StenGine
{

class D3D11SRVBinding
{
public:
	D3D11SRVBinding();
	void AddSRV(ID3D11ShaderResourceView* srv, uint32_t index);
	void Bind();
	void Unbind();

private:
	std::array<ID3D11ShaderResourceView*, 16> m_srvs;
};

}