#pragma once

#include <stdint.h>
#include <vector>

template <class Impl>
class ConstantBufferBase
{
public:

	void* GetBuffer()
	{
		return impl().GetBuffer();
	}

	void Bind()
	{
		impl().Bind();
	}

private:
	Impl &impl() { return *static_cast<Impl *>(this); }


};