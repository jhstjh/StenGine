#include "GLConstantBuffer.h"
#include <malloc.h>
#include <string.h>

#include "gl\glew.h"

GLConstantBuffer::GLConstantBuffer(uint32_t index, uint32_t size, int32_t bufferName)
	: m_index(index)
	, m_bufferName(bufferName)
	, m_size(size)
{
	m_data.resize(size);
}

GLConstantBuffer::~GLConstantBuffer()
{
	
}

void *GLConstantBuffer::GetBuffer()
{
	return m_data.data();
}

void GLConstantBuffer::Bind()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_index, m_bufferName);
	void* ubo = glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		m_size,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
		);
	memcpy(ubo, m_data.data(), m_size);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}
