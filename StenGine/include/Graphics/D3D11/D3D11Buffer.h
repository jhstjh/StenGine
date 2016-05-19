#pragma once

#include "Graphics/D3DIncludes.h"
#include <stdint.h>
#include "Graphics/Abstraction/GPUBufferBase.h"

namespace StenGine
{

class D3D11Buffer
{
public:
	D3D11Buffer(size_t size, BufferUsage usage, void* data = nullptr, BufferType type = BufferType::GENERAL);
	~D3D11Buffer();

	void* map();
	void unmap();

	ID3D11Buffer* GetBuffer() { return m_buffer; };

private:
	ID3D11Buffer* m_buffer;
	size_t m_size;
	uint32_t m_usage;
	uint32_t m_flags;
	bool m_mapped;
};

using GPUBuffer = D3D11Buffer;

}