#pragma once

#include <stdint.h>
#include <vector>

class GLConstantBuffer
{
public:
	GLConstantBuffer() = delete;
	GLConstantBuffer(uint32_t index, uint32_t size, int32_t bufferName);
	~GLConstantBuffer();

	void* GetBuffer();
	void Bind();

private:
	std::vector<uint8_t> m_data;
	uint32_t m_index;
	int32_t m_bufferName;
	uint32_t m_size;
};