// StenGine.cpp : Defines the entry point for the application.
//

#include "windowsX.h"
#include "stdafx.h"
#include "StenGine.h"
#include "D3D11Renderer.h"
#include "EffectsManager.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
//#include "MathHelper.h"
#include "Timer.h"

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
BOOL				InitInstance(HINSTANCE hInstance, int nCmdShow, HWND &hMainWnd);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg = { 0 };

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_STENGINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow, hMainWnd))
	{
		return FALSE;
	}

	D3D11Renderer* renderer = new D3D11Renderer(hInstance, hMainWnd);
 	if (!renderer->Init())
 	{
 		return FALSE;
 	}

	EffectsManager::Instance();
	CameraManager::Instance();
	ResourceManager::Instance();

	//same mesh render with instancing
	GameObject* box0 = new GameObject();
	Mesh* box0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"GenerateBox");
	box0->AddComponent(box0Mesh);
	box0->RotateAroundY(3.14159 / 5);

// 	GameObject* sphere = new GameObject(0, 2.5, -0.5);
// 	Mesh* sphereMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/earth.fbx");
// 	sphere->AddComponent(sphereMesh);
// 
 	GameObject* plane0 = new GameObject(-1, 0, 0);
	Mesh* plane0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"GeneratePlane");
 	plane0->AddComponent(plane0Mesh);
	plane0->RotateAroundY(3.14159);
// 
// 	GameObject* dragon = new GameObject(-3, -1, 0);
// 	Mesh* dragonMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/dragon.fbx");
// 	dragon->AddComponent(dragonMesh);

	Timer::Init();

	float elapsedTime = 0;
	UINT elaspedFrame = 0;
	while (msg.message != WM_QUIT) {
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			Timer::Update();
			elapsedTime += Timer::GetDeltaTime();
			if (elapsedTime >= 1) {
				elapsedTime -= 1;
				FPS = elaspedFrame;
				
				std::wostringstream outs;
				outs.precision(6);
				outs << L"StenGine    "
					<< L"FPS: " << FPS << L"    ";
				SetWindowText(hMainWnd, outs.str().c_str());
				elaspedFrame = 0;
			}
			//sphere->Update();
			D3D11Renderer::Instance()->Draw();
			elaspedFrame++;
		}
	}

	SafeDelete(box0);
// 	SafeDelete(sphere);
 	SafeDelete(plane0);
// 	SafeDelete(dragon);

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

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND &hMainWnd)
{
	RECT wr = { 0, 0, 1280, 720 };    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

   hInst = hInstance; // Store instance handle in our global variable

   hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

   if (!hMainWnd)
   {
      return FALSE;
   }

   ShowWindow(hMainWnd, nCmdShow);
   UpdateWindow(hMainWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
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
		CameraManager::Instance()->GetActiveCamera()->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_RBUTTONUP:
		CameraManager::Instance()->GetActiveCamera()->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		CameraManager::Instance()->GetActiveCamera()->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
