// StenGine.cpp : Defines the entry point for the application.
//
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "windowsX.h"
#include "stdafx.h"
#include "StenGine.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/UI/ImGuiMenu.h"
#include "Scene/LightManager.h"
#include "Scene/CameraManager.h"
#include "Resource/ResourceManager.h"
#include "Scene/GameObjectManager.h"
#include "Input/InputManager.h"
#include "Utility/Timer.h"
#include <memory>
#include <crtdbg.h>

#pragma warning(disable:4996)

#define MAX_LOADSTRING 100

using namespace StenGine;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hMainWnd;
std::wstring mMainWndCaption;
float FPS;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				CreateWindowInstance(int32_t w, int32_t h, HINSTANCE hInstance/*, int nCmdShow*/, HWND &hMainWnd);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	/*******create console window**************/
	AllocConsole();

	// redirect unbuffered STDOUT to the console
	freopen("CONOUT$", "w+t", stdout);

	// redirect unbuffered STDIN to the console
	freopen("CONIN$", "r+t", stdin);

	// redirect unbuffered STDERR to the console
	freopen("CONOUT$", "w+t", stderr);

	std::ios::sync_with_stdio();

	/*********************************/

	MSG msg = { 0 };

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_STENGINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	Renderer* renderer = Renderer::Create(hInstance, hMainWnd);
	renderer->Init(1280, 720, CreateWindowInstance);

	std::unique_ptr<EffectsManager> effectManager = std::unique_ptr<EffectsManager>(EffectsManager::Instance()) ;
	std::unique_ptr<CameraManager> cameraManager = std::unique_ptr<CameraManager>(CameraManager::Instance());
	std::unique_ptr<ResourceManager> resourceManager = std::unique_ptr<ResourceManager>(ResourceManager::Instance());
	std::unique_ptr<InputManager> inputManager = std::unique_ptr<InputManager>(InputManager::Instance());
	std::unique_ptr<GameObjectManager> gameObjectManager = std::unique_ptr<GameObjectManager>(GameObjectManager::Instance());
	std::unique_ptr<ImGuiMenu> imguiMenu = std::unique_ptr<ImGuiMenu>(ImGuiMenu::Instance());

	gameObjectManager->LoadScene();

	Timer::Init();

	float elapsedTime = 0;
	UINT elaspedFrame = 0;

	bool appIsRunning = false;

	while (!(appIsRunning && msg.message == WM_QUIT)) {
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			appIsRunning = true;
			Timer::Update();
			elapsedTime += Timer::GetDeltaTime();
			if (elapsedTime >= 1) {
				elapsedTime -= 1;
				FPS = (float)elaspedFrame;
				
				std::ostringstream outs;
				outs.precision(6);
				outs << "StenGine    "
					<< "FPS: " << FPS << "    ";
				renderer->UpdateTitle(outs.str().c_str());
				elaspedFrame = 0;
			}
			inputManager->Update();
			cameraManager->GetActiveCamera()->Update();			
			gameObjectManager->Update();
			renderer->Draw();
			elaspedFrame++;
		}
	}

	SafeRelease(renderer);

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STENGINE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL CreateWindowInstance(int32_t w, int32_t h, HINSTANCE hInstance/*, int nCmdShow*/, HWND &hMainWnd)
{
	RECT wr = { 0, 0, w, h };    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

	hInst = hInstance; // Store instance handle in our global variable

	hMainWnd = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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
