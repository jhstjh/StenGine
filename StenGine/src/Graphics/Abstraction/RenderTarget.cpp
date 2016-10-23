#include "Graphics/Abstraction/RenderTarget.h"
#include "Graphics/D3D11/D3D11RenderTarget.h"
#include "Graphics/OpenGL/GLRenderTarget.h"
#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

RenderTarget::RenderTarget()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		mImpl = std::make_unique<D3D11RenderTarget>();
		break;
	case RenderBackend::OPENGL4:
		mImpl = std::make_unique<GLRenderTarget>();
		break;
	}
}

// D3D11 only
void RenderTarget::SetRenderTarget(void* deviceContext)
{
	mImpl->SetRenderTarget(deviceContext);
}

void RenderTarget::ClearColor(void* deviceContext)
{
	mImpl->ClearColor(deviceContext);
}

void RenderTarget::ClearDepth(void* deviceContext)
{
	mImpl->ClearDepth(deviceContext);
}

// GL only
void RenderTarget::Set(uint32_t framebuffer)
{
	mImpl->Set(framebuffer);
}

uint32_t RenderTarget::Get()
{
	return mImpl->Get();
}

void RenderTarget::AddRenderTarget(void* renderTarget)
{
	mImpl->AddRenderTarget(renderTarget);
}

void RenderTarget::AssignDepthStencil(void* depthStencil)
{
	mImpl->AssignDepthStencil(depthStencil);
}

void RenderTarget::AddClearColor(SGColors color)
{
	mImpl->AddClearColor(color);
}

}