#pragma once

#include "ConstantBufferBase.h"
#include <stdint.h>

class GLConstantBuffer : public ConstantBufferBase<GLConstantBuffer>
{
friend class ConstantBufferBase<GLConstantBuffer>;

public:
	GLConstantBuffer(uint32_t index, uint32_t size, void* bufferName);
	~GLConstantBuffer();

	GLConstantBuffer(GLConstantBuffer&& other);
	GLConstantBuffer& operator=(GLConstantBuffer&& other);

protected:
	void* ImplGetBuffer();
	void ImplBind();

private:
	void* m_data;
	uint32_t m_index;
	int32_t m_bufferName;
	uint32_t m_size;
};

using ConstantBuffer = GLConstantBuffer;