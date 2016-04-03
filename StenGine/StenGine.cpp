// StenGine.cpp : Defines the entry point for the application.
//
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "windowsX.h"
#include "stdafx.h"
#include "StenGine.h"
#include "RendererBase.h"
#include "EffectsManager.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "InputManager.h"
#include "Terrain.h"
//#include "MathHelper.h"
#include "Timer.h"
#include <memory>

#pragma warning(disable:4996)

#define MAX_LOADSTRING 100

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

	GameObject* box0 = new GameObject("box0", 0.f, 1.2f, 0.f);
	Mesh* box0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"GenerateBox");
	box0->AddComponent(box0Mesh);
	box0->RotateAroundY(3.14159f / 5);

	GameObject* sphere = new GameObject("sphere", 0.f, 3.7f, -0.5f);
	Mesh* sphereMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/earth.fbx");
	sphere->AddComponent(sphereMesh);
	
	GameObject* plane0 = new GameObject("plane0", 4.f, 0.2f, 0.f);
	Mesh* plane0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plane.fbx");
	plane0->AddComponent(plane0Mesh);
	
	GameObject* plane1 = new GameObject("plane1", -4.f, 0.2f, 0.f);
	Mesh* plane1Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plane.fbx");
	plane1->AddComponent(plane1Mesh);

	GameObject* plants0 = new GameObject("plants0", -4.f, 0.2f, 0.f);
	Mesh* plants0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plants.fbx");
	plants0->AddComponent(plants0Mesh);

	GameObject* house0 = new GameObject("plants0", 0.f, -0.1f, 20.f);
	Mesh* house0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/house.fbx");
	house0->AddComponent(house0Mesh);
	house0->RotateAroundY(3.1415926f / 2);


// 	GameObject* dragon = new GameObject(-3, -1, 0);
// 	Mesh* dragonMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/dragon.fbx");
// 	dragon->AddComponent(dragonMesh);
#ifdef GRAPHICS_D3D11
	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Terrain/terrain.raw";
	tii.LayerMapFilenames.resize(5);
	tii.LayerMapFilenames[0] = L"Terrain/grass.dds";
	tii.LayerMapFilenames[1] = L"Terrain/darkdirt.dds";
	tii.LayerMapFilenames[2] = L"Terrain/stone.dds";
	tii.LayerMapFilenames[3] = L"Terrain/lightdirt.dds";
	tii.LayerMapFilenames[4] = L"Terrain/snow.dds";
	tii.BlendMapFilename = L"Terrain/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;

	GameObject* terrain = new GameObject("Terrain", 0, 0, -100);


	Terrain* terrainComp = new Terrain(tii);
	terrain->AddComponent(terrainComp);
#endif
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
				//SetWindowText(hMainWnd, outs.str().c_str());
				renderer->UpdateTitle(outs.str().c_str());
				elaspedFrame = 0;
			}
			InputManager::Instance()->Update();
			CameraManager::Instance()->GetActiveCamera()->Update();			
			sphere->Update();
			Renderer::Instance()->Draw();
			elaspedFrame++;
		}
	}

	SafeRelease(renderer);

//  	SafeDelete(box0);
//   	SafeDelete(sphere);
//   	SafeDelete(plane0);
//	SafeDelete(dragon);

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
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
	//int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_RBUTTONDOWN:
//		CameraManager::Instance()->GetActiveCamera()->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_RBUTTONUP:
//		CameraManager::Instance()->GetActiveCamera()->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
//		CameraManager::Instance()->GetActiveCamera()->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
