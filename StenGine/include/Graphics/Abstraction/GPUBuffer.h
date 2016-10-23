#pragma once

#include <memory>

namespace StenGine
{

enum class BufferUsage
{
	IMMUTABLE,
	DYNAMIC,
	WRITE,
};

enum class BufferType
{
	GENERAL = 0x00L,
	VERTEX_BUFFER = 0x01L,
	INDEX_BUFFER = 0x02L,
	CONSTANT_BUFFER = 0x04L,
	SHADER_RESOURCE = 0x08L,
	UNORDERED_ACCESS = 0x80L,
};

class GPUBufferImpl
{
public:
	virtual ~GPUBufferImpl() = default;
	virtual void* map() = 0;
	virtual void  unmap() = 0;
	virtual void* GetBuffer() = 0;
	virtual uint32_t GetFlags() = 0;
};


class GPUBuffer
{
public:
	GPUBuffer(size_t size, BufferUsage usage, void* data = nullptr, BufferType type = BufferType::GENERAL);
	~GPUBuffer();

	void* map();
	void unmap();
	void* GetBuffer();
	uint32_t GetFlags();
private:
	std::unique_ptr<GPUBufferImpl> mImpl;
};


}