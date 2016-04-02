#ifndef __CONSTANT_BUFFER_H_
#define __CONSTANT_BUFFER_H_

#include <stdint.h>

struct ConstantBuffer
{
	uint32_t offset;
	uint32_t size;
	uint32_t pos;
	void* data;

	ConstantBuffer(uint32_t _offset, uint32_t _size, uint32_t _pos, void* _data)
		: offset(_offset)
		, size(_size)
		, pos(_pos)
		, data(_data) 
	{ }
};

#endif