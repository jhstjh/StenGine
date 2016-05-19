#ifndef __GL_BUFFER_H_
#define __GL_BUFFER_H_

#include "glew.h"
#include <stdint.h>

#include "Graphics/Abstraction/GPUBufferBase.h"

namespace StenGine
{

class GLBuffer
{
public:
	GLBuffer(size_t size, BufferUsage usage, void* data = nullptr, BufferType type = BufferType::GENERAL);
	~GLBuffer();

	void* map();
	void unmap();

	GLuint GetBuffer() { return m_buffer; }
	uint32_t GetFlags() { return m_flags; }

private:
	GLuint m_buffer;
	size_t m_size;
	uint32_t m_flags;
	bool m_mapped;
};

using GPUBuffer = GLBuffer;

}
#endif