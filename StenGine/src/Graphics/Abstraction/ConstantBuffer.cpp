#include "Graphics/Abstraction/ConstantBuffer.h"
#include "Graphics/D3D11/D3D11ConstantBuffer.h"
#include "Graphics/OpenGL/GLConstantBuffer.h"
#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

ConstantBuffer::ConstantBuffer(uint32_t index, uint32_t size, void* bufferName)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		mImpl = std::make_unique<D3D11ConstantBuffer>(index, size, bufferName);
		break;
	case RenderBackend::OPENGL4:
		mImpl = std::make_unique<GLConstantBuffer>(index, size, bufferName);
		break;
	}
}

ConstantBuffer::~ConstantBuffer()
{
	mImpl = nullptr;
}

ConstantBuffer::ConstantBuffer(ConstantBuffer &&other)
{
	mImpl = std::move(other.mImpl);
	other.mImpl = nullptr;
}

ConstantBuffer& ConstantBuffer::operator=(ConstantBuffer&& other)
{
	mImpl = std::move(other.mImpl);
	other.mImpl = nullptr;
	return *this;
}

void* ConstantBuffer::GetBuffer()
{
	return mImpl->GetBuffer();
}

void ConstantBuffer::Bind()
{
	mImpl->Bind();
}

}