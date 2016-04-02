#ifndef __GL_BUFFER_H_
#define __GL_BUFFER_H_

#include "GL/glew.h"
#include <stdint.h>

class GLBuffer
{
public:
	GLBuffer(uint32_t size);
	~GLBuffer();

	void* lock();
	void unlock();

	GLuint GetBuffer() { return m_buffer; };

private:
	GLuint m_buffer;
	uint32_t m_size;
	bool m_mapped;
	uint32_t m_flags;
};

#endif