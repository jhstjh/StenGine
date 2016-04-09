#include "Graphics/OpenGL/GLBuffer.h"

GLBuffer::GLBuffer(uint32_t size)
	: m_size(size)
	, m_mapped(false)
{
	m_flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;

	glCreateBuffers(1, &m_buffer);
	glNamedBufferStorageEXT(m_buffer, size, nullptr, m_flags);
}

GLBuffer::~GLBuffer()
{
	glDeleteBuffers(1, &m_buffer);
}

void * GLBuffer::lock()
{
	m_mapped = true;
	return glMapNamedBufferRangeEXT(m_buffer, 0, m_size, m_flags);
}

void GLBuffer::unlock()
{
	m_mapped = false;
	glUnmapNamedBufferEXT(m_buffer);
}
