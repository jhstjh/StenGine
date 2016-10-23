#include "Graphics/Abstraction/UAVBinding.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3D11/D3D11UAVBinding.h"
#include "Graphics/OpenGL/GLUAVBinding.h"

namespace StenGine
{

UAVBinding::UAVBinding()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		mImpl = std::make_unique<D3D11UAVBinding>();
		break;
	case RenderBackend::OPENGL4:
		mImpl = std::make_unique<GLUAVBinding>();
		break;
	}
}

void UAVBinding::AddUAV(void* UAV, uint32_t index)
{
	mImpl->AddUAV(UAV, index);
}

void UAVBinding::Bind()
{
	mImpl->Bind();
}

void UAVBinding::Unbind()
{
	mImpl->Unbind();
}

}