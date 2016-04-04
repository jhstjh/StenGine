#include "D3D11SRVBinding.h"
#include "RendererBase.h"

#pragma warning(disable:4267)

D3D11SRVBinding::D3D11SRVBinding()
{
	m_srvs.fill(nullptr);
}

void D3D11SRVBinding::AddSRV(ID3D11ShaderResourceView * srv, uint32_t index)
{
	m_srvs[index] = srv;
}

void D3D11SRVBinding::Bind()
{
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 5, &m_srvs[0]); // todo 0, 5
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 5, &m_srvs[0]);
}

void D3D11SRVBinding::Unbind()
{
	static ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 16, nullSRV);
}