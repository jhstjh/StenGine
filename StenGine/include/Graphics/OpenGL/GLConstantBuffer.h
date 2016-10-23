#pragma once

#include "Graphics/Abstraction/ConstantBuffer.h"
#include <stdint.h>

namespace StenGine
{

class GLConstantBuffer : public ConstantBufferImpl
{
public:
	GLConstantBuffer(uint32_t index, uint32_t size, void* bufferName);
	~GLConstantBuffer();

	virtual void* GetBuffer();
	virtual void Bind();

private:
	void* m_data;
	uint32_t m_index;
	int32_t m_bufferName;
	uint32_t m_size;
};

}