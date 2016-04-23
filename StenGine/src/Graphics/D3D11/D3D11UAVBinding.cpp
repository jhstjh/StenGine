#include "Graphics/D3D11/D3D11UAVBinding.h"
#include "Graphics/Abstraction/RendererBase.h"

#pragma warning(disable:4267)

namespace StenGine
{

D3D11UAVBinding::D3D11UAVBinding()
{
	m_UAVs.fill(nullptr);
}

void D3D11UAVBinding::AddUAV(ID3D11UnorderedAccessView * UAV, uint32_t index)
{
	m_UAVs[index] = UAV;
}

void D3D11UAVBinding::Bind()
{
	// TODO only set shader in interest
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetUnorderedAccessViews(0, 16, &m_UAVs[0], 0); // todo 0, 5
}

void D3D11UAVBinding::Unbind()
{
	// TODO only set shader in interest
	static ID3D11UnorderedAccessView* nullUAV[16] = { 0 };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetUnorderedAccessViews(0, 16, nullUAV, 0);
}

}