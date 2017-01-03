#include "Graphics/Abstraction/GPUBuffer.h"
#include "Graphics/D3D11/D3D11Buffer.h"
#include "Graphics/OpenGL/GLBuffer.h"
#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

GPUBuffer::GPUBuffer(size_t size, BufferUsage usage, void* data/* = nullptr*/, BufferType type/* = BufferType::GENERAL*/)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		mImpl = std::make_unique<D3D11Buffer>(size, usage, data, type);
		break;
	case RenderBackend::OPENGL4:
		mImpl = std::make_unique<GLBuffer>(size, usage, data, type);
		break;
	}
}

GPUBuffer::~GPUBuffer()
{

}

void* GPUBuffer::map()
{
	return mImpl->map();
}

void GPUBuffer::unmap() 
{
	mImpl->unmap();
}

void* GPUBuffer::GetBuffer()
{
	return mImpl->GetBuffer();
}

uint32_t GPUBuffer::GetFlags()
{
	return mImpl->GetFlags();
}

void GPUBuffer::bind(uint32_t bindpoint)
{
	mImpl->bind(bindpoint);
}


}