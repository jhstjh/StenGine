#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3D11/D3D11Renderer.h"

namespace StenGine
{

Renderer* Renderer::_instance = nullptr;

Renderer* Renderer::Create(HINSTANCE hInstance, HWND hMainWnd)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11Renderer* renderer = new D3D11Renderer(hInstance, hMainWnd);
		_instance = static_cast<Renderer*>(renderer);
		break;
	}
	case RenderBackend::OPENGL4:
		//	mImpl = std::make_unique<GLConstantBuffer>(index, size, bufferName);
		break;
	}
	return _instance;
}

}