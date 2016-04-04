#pragma once

#include "D3DIncludes.h"
#include <array>

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