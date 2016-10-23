#include "glew.h"
#include "Graphics/OpenGL/GLConstantBuffer.h"
#include <malloc.h>
#include <string.h>

#pragma warning(disable: 4311  4302)

namespace StenGine
{

GLConstantBuffer::GLConstantBuffer(uint32_t index, uint32_t size, void* bufferName)
{
	m_index = index;
	m_bufferName = (uint32_t)bufferName;
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
	glBindBufferRange(GL_UNIFORM_BUFFER, m_index, m_bufferName, 0, m_size);
	void* ubo = glMapNamedBufferRange(
		m_bufferName,
		0,
		m_size,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
	);
	memcpy(ubo, m_data, m_size);
	glUnmapNamedBuffer(m_bufferName);
}

}