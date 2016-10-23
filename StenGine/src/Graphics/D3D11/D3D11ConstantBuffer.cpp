#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3D11/D3D11ConstantBuffer.h"

#include <malloc.h>
#include <string.h>

namespace StenGine
{

D3D11ConstantBuffer::D3D11ConstantBuffer(uint32_t index, uint32_t size, void* buffer)
	: m_bufferName((ID3D11Buffer*)buffer)
	, m_index(index)
	, m_size(size)
{
	m_data = _aligned_malloc(m_size, 16);
}

D3D11ConstantBuffer::~D3D11ConstantBuffer()
{
	if (m_data)
	{
		_aligned_free(m_data);
		m_data = 0;
	}
}

void *D3D11ConstantBuffer::GetBuffer()
{
	return m_data;
}

void D3D11ConstantBuffer::Bind()
{
	D3D11_MAPPED_SUBRESOURCE ms;
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_bufferName, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, m_data, m_size);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_bufferName, NULL);

	// TODO only set shader in interest
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(m_index, 1, &m_bufferName);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(m_index, 1, &m_bufferName);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetConstantBuffers(m_index, 1, &m_bufferName);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetConstantBuffers(m_index, 1, &m_bufferName);
}

}