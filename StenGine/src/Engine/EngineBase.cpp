#include "Engine/EngineBase.h"
#include "Resource.h"
#include "Utility/CommandlineParser.h"

namespace StenGine
{

EngineBase::EngineBase()
{

}

EngineBase::~EngineBase()
{
	SafeRelease(m_renderer);
}


BOOL EngineBase::CreateWindowInstance(int32_t w, int32_t h, HINSTANCE hInstance/*, int nCmdShow*/, HWND &hMainWnd)
{
	RECT wr = { 0, 0, w, h };    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

	m_hInstance = hInstance; // Store instance handle in our global variable

	hMainWnd = CreateWindowEx(WS_EX_CLIENTEDGE, m_szWindowClass, m_szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	if (!hMainWnd)
	{
		return FALSE;
	}

	UpdateWindow(hMainWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (StenGine::ImGuiMenu::Created())
		StenGine::ImGuiMenu::Instance()->HandleMsg(message, wParam, lParam);

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM EngineBase::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STENGINE));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = m_szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

void EngineBase::Init(HINSTANCE hInstance)
{
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_STENGINE, m_szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	auto backend = CommandlineParser::Instance()->GetCommand(L"-g");
	if (_wcsicmp(backend, L"d3d11") == 0)
	{
		Renderer::SetRenderBackend(RenderBackend::D3D11);
	}
	else if (_wcsicmp(backend, L"gl4") == 0)
	{
		Renderer::SetRenderBackend(RenderBackend::OPENGL4);
	}

	m_console = std::make_unique<Console>();
	m_eventSystem = std::unique_ptr<EventSystem>(EventSystem::Instance());

	m_renderer = Renderer::Create(hInstance, m_hMainWnd);

	auto func = [this](int32_t w, int32_t h, HINSTANCE hInstance, /*int32_t,*/ HWND& hMainWnd) -> BOOL
	{
		return CreateWindowInstance(w, h, hInstance, hMainWnd);
	};

	m_renderer->Init(1280, 720, func);

	m_effectManager = std::unique_ptr<EffectsManager>(EffectsManager::Instance());
	m_cameraManager = std::unique_ptr<CameraManager>(CameraManager::Instance());
	m_resourceManager = std::unique_ptr<ResourceManager>(ResourceManager::Instance());
	m_inputManager = std::unique_ptr<InputManager>(InputManager::Instance());
	m_gameObjectManager = std::unique_ptr<GameObjectManager>(GameObjectManager::Instance());
	m_imguiMenu = std::unique_ptr<ImGuiMenu>(ImGuiMenu::Instance());

	m_gameObjectManager->LoadScene();

	Timer::Init();
}

void EngineBase::Run()
{
	MSG msg = { 0 };
	while (!(m_appIsRunning && msg.message == WM_QUIT)) {
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			m_appIsRunning = true;
			Timer::Update();
			m_elapsedTime += Timer::GetDeltaTime();
			if (m_elapsedTime >= 1) {
				m_elapsedTime -= 1;
				m_fps = (float)m_elaspedFrame;

				std::ostringstream outs;
				outs.precision(6);
				outs << "StenGine    "
					<< "FPS: " << m_fps << "    ";
				m_renderer->UpdateTitle(outs.str().c_str());
				m_elaspedFrame = 0;
			}

			m_eventSystem->Update();

			m_elaspedFrame++;
		}
	}
}

}