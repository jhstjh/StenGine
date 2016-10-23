#pragma once
#include <memory>

namespace StenGine
{

class UAVBindingImpl
{
public:
	virtual ~UAVBindingImpl() = default;
	virtual void AddUAV(void* UAV, uint32_t index) = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};

class UAVBinding
{
public:
	UAVBinding();
	void AddUAV(void* UAV, uint32_t index);
	void Bind();
	void Unbind();

private:
	std::unique_ptr<UAVBindingImpl> mImpl;
};


}