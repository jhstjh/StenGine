#pragma once

#include "Graphics/Abstraction/ConstantBuffer.h"
#include "Graphics/Abstraction/GPUBuffer.h"
#include <stdint.h>

namespace StenGine
{

class GLConstantBuffer : public ConstantBufferImpl
{
public:
	GLConstantBuffer(uint32_t index, uint32_t size, GPUBuffer* buffer);
	~GLConstantBuffer();

	virtual void* GetBuffer();
	virtual void Bind();

private:
	void* m_data;
	uint32_t m_index;
	GPUBuffer* m_buffer;
	uint32_t m_size;
};

}