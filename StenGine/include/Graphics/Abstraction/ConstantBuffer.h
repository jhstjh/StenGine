#pragma once
#include <memory>

namespace StenGine
{

class ConstantBufferImpl
{
public:
	virtual ~ConstantBufferImpl() = default;
	virtual void* GetBuffer() = 0;
	virtual void Bind() = 0;
};

class ConstantBuffer
{
public:
	ConstantBuffer(uint32_t index, uint32_t size, class GPUBuffer* buffer);
	ConstantBuffer(ConstantBuffer &&other);
	ConstantBuffer& operator=(ConstantBuffer&& other);
	~ConstantBuffer();

	void* GetBuffer();
	void Bind();
private:
	std::unique_ptr<ConstantBufferImpl> mImpl;
};

}