#include "Graphics/D3D11/D3D11Buffer.h"
#include "Graphics/Abstraction/RendererBase.h"
#include <assert.h>

namespace StenGine
{

D3D11Buffer::D3D11Buffer(size_t size, BufferUsage usage, void* data, BufferType type)
	: m_size(size)
	, m_mapped(false)
	, m_flags(0)
	, m_type(type)
{
	uint32_t misc = 0;
	uint32_t stride = 0;

	switch (usage)
	{
	case BufferUsage::IMMUTABLE:
		m_usage = D3D11_USAGE_IMMUTABLE;
		break;
	case BufferUsage::DYNAMIC:
		m_usage = D3D11_USAGE_DYNAMIC;
		break;
	case BufferUsage::WRITE:
		m_usage = D3D11_USAGE_DYNAMIC;
		m_flags = D3D11_CPU_ACCESS_WRITE;
		break;
	default:
		m_usage = D3D11_USAGE_DEFAULT;
		break;
	}

	const uint32_t d3d11BufferBindFlag[] =
	{
		0, 
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_BIND_INDEX_BUFFER,
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_BIND_SHADER_RESOURCE,
		D3D11_BIND_UNORDERED_ACCESS,
	};

	D3D11_BUFFER_DESC desc;
	desc.Usage = (D3D11_USAGE)m_usage;
	desc.ByteWidth = size;
	desc.BindFlags = (UINT)d3d11BufferBindFlag[(uint32_t)type];
	desc.CPUAccessFlags = m_flags;
	desc.MiscFlags = misc;
	desc.StructureByteStride = stride;
	D3D11_SUBRESOURCE_DATA vinitData;

	if (data)
		vinitData.pSysMem = data;
	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&desc, data ? &vinitData : nullptr, &m_buffer));
}

D3D11Buffer::~D3D11Buffer()
{
	ReleaseCOM(m_buffer);
}

void* D3D11Buffer::map()
{
	m_mapped = true;
	D3D11_MAPPED_SUBRESOURCE ms;
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	return ms.pData;
}

void D3D11Buffer::unmap()
{
	m_mapped = false;
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_buffer, NULL);

}

void D3D11Buffer::bind(uint32_t bindpoint)
{
	switch (m_type)
	{
	case StenGine::BufferType::CONSTANT_BUFFER:
		// TODO only set shader in interest
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(bindpoint, 1, &m_buffer);
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(bindpoint, 1, &m_buffer);
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetConstantBuffers(bindpoint, 1, &m_buffer);
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetConstantBuffers(bindpoint, 1, &m_buffer);
		break;
	default:
		assert(0);
		break;
	}
}

}
