#include "Graphics/D3D11/D3D11Buffer.h"
#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

D3D11Buffer::D3D11Buffer(size_t size, BufferUsage usage, void* data, BufferType type)
	: m_size(size)
	, m_mapped(false)
	, m_flags(0)
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

	D3D11_BUFFER_DESC desc;
	desc.Usage = (D3D11_USAGE)m_usage;
	desc.ByteWidth = size;
	desc.BindFlags = (UINT)type;
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

}
