#include "stdafx.h"

#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

Renderer* Renderer::_instance = nullptr;
RenderBackend Renderer::_backend = RenderBackend::D3D11;

extern Renderer* CreateD3D11Renderer(HINSTANCE hInstance, HWND hMainWnd);
extern Renderer* CreateGLRenderer(HINSTANCE hInstance, HWND hMainWnd, Semaphore &prepareDrawListSync, Semaphore &finishedDrawListSync);

std::unique_ptr<Renderer> Renderer::Create(HINSTANCE hInstance, HWND hMainWnd, Semaphore &prepareDrawListSync, Semaphore &finishedDrawListSync)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		_instance = CreateD3D11Renderer(hInstance, hMainWnd);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		_instance = CreateGLRenderer(hInstance, hMainWnd, prepareDrawListSync, finishedDrawListSync);
		break;
	}
	}

	return std::move(std::unique_ptr<Renderer>(_instance));
}

}