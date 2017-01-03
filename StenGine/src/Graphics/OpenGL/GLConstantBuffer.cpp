#include "glew.h"
#include "Graphics/OpenGL/GLConstantBuffer.h"
#include <malloc.h>
#include <string.h>
#include <assert.h>

#pragma warning(disable: 4311  4302)

namespace StenGine
{

GLConstantBuffer::GLConstantBuffer(uint32_t index, uint32_t size, GPUBuffer* buffer)
{
	m_index = index;
	m_buffer = buffer;
	m_size = size;

	m_data = _aligned_malloc(m_size, 16);
}

GLConstantBuffer::~GLConstantBuffer()
{
	if (m_data)
		_aligned_free(m_data);
}

void *GLConstantBuffer::GetBuffer()
{
	return m_data;
}

void GLConstantBuffer::Bind()
{
	m_buffer->bind(m_index);
	void* ubo = m_buffer->map();
	assert(ubo);
	memcpy(ubo, m_data, m_size);
	m_buffer->unmap();
}

}