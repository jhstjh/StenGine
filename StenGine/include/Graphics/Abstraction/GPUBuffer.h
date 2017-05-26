#pragma once

#include <memory>

namespace StenGine
{

enum class BufferUsage
{
	IMMUTABLE,
	DYNAMIC,
	WRITE,
	DEFAULT,
};

enum class BufferType
{
	GENERAL,
	VERTEX_BUFFER,
	INDEX_BUFFER,
	CONSTANT_BUFFER,
	SHADER_RESOURCE,
	UNORDERED_ACCESS,

	SSBO, // opengl specific
};

class GPUBufferImpl
{
public:
	virtual ~GPUBufferImpl() = default;
	virtual void* map() = 0;
	virtual void  unmap() = 0;
	virtual void* GetBuffer() = 0;
	virtual uint32_t GetFlags() = 0;
	virtual void bind(uint32_t bindpoint) = 0;
};

using GPUBuffer = std::shared_ptr<GPUBufferImpl>;

}