#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3D11/D3D11Renderer.h"
#include "Graphics/OpenGL/GLRenderer.h"

namespace StenGine
{

Renderer* Renderer::_instance = nullptr;
RenderBackend Renderer::_backend = RenderBackend::D3D11;

std::unique_ptr<Renderer> Renderer::Create(HINSTANCE hInstance, HWND hMainWnd)
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
	{
		GLRenderer* renderer = new GLRenderer(hInstance, hMainWnd);
		_instance = static_cast<Renderer*>(renderer);
		break;
	}
	}

	return std::move(std::unique_ptr<Renderer>(_instance));
}

}