#pragma once

#include "Graphics/Abstraction/UAVBinding.h"
#include "Graphics/D3DIncludes.h"
#include <array>

namespace StenGine
{

class D3D11UAVBinding : public UAVBindingImpl
{
public:
	D3D11UAVBinding();
	virtual void AddUAV(void* UAV, uint32_t index);
	virtual void Bind();
	virtual void Unbind();

private:
	std::array<ID3D11UnorderedAccessView*, 16> m_UAVs;
};

}