#pragma once
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/UI/ImGuiMenu.h"
#include "Scene/LightManager.h"
#include "Scene/CameraManager.h"
#include "Resource/ResourceManager.h"
#include "Scene/GameObjectManager.h"
#include "Input/InputManager.h"
#include "Utility/Timer.h"
#include "Engine/Console.h"
#include "Engine/EventSystem.h"
#include "Mesh/Terrain.h"

#include <Windows.h>


namespace StenGine
{

#define MAX_LOADSTRING 100

class EngineBase
{
public:
	EngineBase();
	~EngineBase();

	void Init(HINSTANCE hInstance);
	void Run();

	virtual void GameInit() = 0;

protected:
	ATOM MyRegisterClass(HINSTANCE hInstance);
	BOOL CreateWindowInstance(int32_t w, int32_t h, HINSTANCE hInstance/*, int nCmdShow*/, HWND &hMainWnd);

	TCHAR m_szTitle[MAX_LOADSTRING];					// The title bar text
	TCHAR m_szWindowClass[MAX_LOADSTRING];			// the main window class name

	HINSTANCE m_hInstance;
	HWND m_hMainWnd;

	std::unique_ptr<EventSystem> m_eventSystem;
	std::unique_ptr<EffectsManager> m_effectManager;
	std::unique_ptr<CameraManager> m_cameraManager;
	std::unique_ptr<ResourceManager> m_resourceManager;
	std::unique_ptr<InputManager> m_inputManager;
	std::unique_ptr<GameObjectManager> m_gameObjectManager;
	std::unique_ptr<ImGuiMenu> m_imguiMenu;
	std::unique_ptr<Console> m_console;

	std::unique_ptr<Renderer> m_renderer;

	float m_fps;
	float m_elapsedTime = 0;
	UINT m_elaspedFrame = 0;

	bool m_appIsRunning = false;
};

}