#include "glew.h"
#include "Graphics/OpenGL/GLConstantBuffer.h"
#include <malloc.h>
#include <string.h>

#pragma warning(disable: 4311  4302)

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

GLConstantBuffer::GLConstantBuffer(GLConstantBuffer&& other)
{
	m_index = other.m_index;
	m_bufferName = other.m_bufferName;
	m_size = other.m_size;
	m_data = other.m_data;

	other.m_data = nullptr;
}

GLConstantBuffer& GLConstantBuffer::operator=(GLConstantBuffer&& other)
{
	if (this != &other)
	{
		if (m_data)
			_aligned_free(m_data);

		m_index = other.m_index;
		m_bufferName = other.m_bufferName;
		m_size = other.m_size;
		m_data = other.m_data;

		other.m_data = nullptr;
	}
	return *this;
}

void *GLConstantBuffer::ImplGetBuffer()
{
	return m_data;
}

void GLConstantBuffer::ImplBind()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_index, m_bufferName);
	void* ubo = glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		m_size,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
		);
	memcpy(ubo, m_data, m_size);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}
