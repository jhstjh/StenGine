#include "Graphics/OpenGL/GLBuffer.h"
#include <assert.h>

namespace StenGine
{

GLBuffer::GLBuffer(size_t size, BufferUsage usage, void* data, BufferType type)
	: m_size(size)
	, m_mapped(false)
	, m_type(type)
{
	switch (usage)
	{
	case BufferUsage::IMMUTABLE:
		m_flags = 0;
		break;
	case BufferUsage::DYNAMIC:
		m_flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
		break;
	case BufferUsage::WRITE:
		m_flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
		break;
	default:
		m_flags = 0;
		break;
	}

	glCreateBuffers(1, &m_buffer);
	glNamedBufferStorageEXT(m_buffer, size, data, m_flags);
}

GLBuffer::~GLBuffer()
{
	glDeleteBuffers(1, &m_buffer);
}

void * GLBuffer::map()
{
	m_mapped = true;
	return glMapNamedBufferRangeEXT(m_buffer, 0, m_size, m_flags);
}

void GLBuffer::unmap()
{
	m_mapped = false;
	glUnmapNamedBufferEXT(m_buffer);
}

void GLBuffer::bind(uint32_t bindpoint)
{
	switch (m_type)
	{
	case BufferType::CONSTANT_BUFFER:
		glBindBufferRange(GL_UNIFORM_BUFFER, bindpoint, m_buffer, 0, m_size);
		break;
	case BufferType::SSBO:
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindpoint, m_buffer, 0, m_size);
		break;
	default:
		assert(0);
		break;
	}
}

}