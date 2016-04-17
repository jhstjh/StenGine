#pragma once

namespace StenGine
{

template <class Impl>
class ConstantBufferBase
{
public:

	void* GetBuffer()
	{
		return impl().ImplGetBuffer();
	}

	void Bind()
	{
		impl().ImplBind();
	}

private:
	Impl &impl() { return *static_cast<Impl *>(this); }


};

}