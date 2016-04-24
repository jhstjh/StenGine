#pragma once

#include "Graphics/D3DIncludes.h"
#include <array>

namespace StenGine
{

class D3D11UAVBinding
{
public:
	D3D11UAVBinding();
	void AddUAV(ID3D11UnorderedAccessView* UAV, uint32_t index);
	void Bind();
	void Unbind();

private:
	std::array<ID3D11UnorderedAccessView*, 16> m_UAVs;
};

using UAVBinding = D3D11UAVBinding;

}