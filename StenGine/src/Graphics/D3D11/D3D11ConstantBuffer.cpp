#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3D11/D3D11ConstantBuffer.h"

#include <malloc.h>
#include <string.h>

namespace StenGine
{

D3D11ConstantBuffer::D3D11ConstantBuffer(uint32_t index, uint32_t size, GPUBuffer buffer)
	: m_index(index)
	, m_size(size)
{
	m_buffer = buffer;

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
	void* cb = m_buffer->map();
	assert(cb);
	memcpy(cb, m_data, m_size);
	m_buffer->unmap();

	m_buffer->bind(m_index);
}

}