#include "Graphics/Abstraction/Texture.h"
#include "Graphics/D3D11/D3D11Texture.h"
#include "Graphics/OpenGL/GLTexture.h"
#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

Texture::Texture(uint32_t width, uint32_t height, void* srv)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		mImpl = std::make_unique<D3D11Texture>(width, height, srv);
		break;
	case RenderBackend::OPENGL4:
		mImpl = std::make_unique<GLTexture>(width, height, srv);
		break;
	}
}

void* Texture::GetTexture()
{
	return mImpl->GetTexture();
}

void Texture::GetDimension(uint32_t &width, uint32_t &height)
{
	mImpl->GetDimension(width, height);
}

}