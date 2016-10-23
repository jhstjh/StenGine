#include "Graphics/UI/ImGuiMenu.h"
#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

namespace detail
{
extern ImGuiMenu* CreateD3D11ImGuiMenu();
extern ImGuiMenu* CreateOpenGLImGuiMenu();
}

ImGuiMenu* ImGuiMenu::_instance = nullptr;

ImGuiMenu* ImGuiMenu::Instance()
{
	if (!_instance)
	{
		switch (Renderer::GetRenderBackend())
		{
		case RenderBackend::D3D11:
			_instance = detail::CreateD3D11ImGuiMenu();
			break;
		case RenderBackend::OPENGL4:
			_instance = detail::CreateOpenGLImGuiMenu();
			break;
		}
	}
	return _instance;
}

}