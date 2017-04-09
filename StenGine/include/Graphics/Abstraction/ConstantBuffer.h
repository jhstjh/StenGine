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

using ConstantBuffer = std::unique_ptr<ConstantBufferImpl>;

}