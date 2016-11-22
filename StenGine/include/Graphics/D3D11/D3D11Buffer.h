#pragma once

#include "Graphics/D3DIncludes.h"
#include <stdint.h>
#include "Graphics/Abstraction/GPUBuffer.h"

namespace StenGine
{

class D3D11Buffer : public GPUBufferImpl
{
public:
	D3D11Buffer(size_t size, BufferUsage usage, void* data = nullptr, BufferType type = BufferType::GENERAL);
	virtual ~D3D11Buffer();

	virtual void* map();
	virtual void unmap();

	virtual void* GetBuffer() { return m_buffer; };
	virtual uint32_t GetFlags() { return m_flags; }

	virtual void bind(uint32_t bindpoint);

private:
	ID3D11Buffer* m_buffer;
	size_t m_size;
	uint32_t m_usage;
	uint32_t m_flags;
	bool m_mapped;
	BufferType m_type;
};

}