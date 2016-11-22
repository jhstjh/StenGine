#ifndef __GL_BUFFER_H_
#define __GL_BUFFER_H_

#include "glew.h"
#include <stdint.h>

#include "Graphics/Abstraction/GPUBuffer.h"

#pragma warning( push )
#pragma warning(disable: 4312) // warning C4312: 'reinterpret_cast': conversion from 'GLuint' to 'void *' of greater size

namespace StenGine
{

class GLBuffer : public GPUBufferImpl
{
public:
	GLBuffer(size_t size, BufferUsage usage, void* data = nullptr, BufferType type = BufferType::GENERAL);
	virtual ~GLBuffer();

	virtual void* map();
	virtual void unmap();

	virtual void* GetBuffer() { return reinterpret_cast<void*>(m_buffer); }
	virtual uint32_t GetFlags() { return m_flags; }

private:
	GLuint m_buffer;
	size_t m_size;
	uint32_t m_flags;
	bool m_mapped;
	BufferUsage m_usage;
	void* m_persistentData;
};

#pragma warning( pop )

}
#endif