#ifndef __D3D11_RENDERER__
#define __D3D11_RENDERER__

#include "stdafx.h"

class D3D11Renderer {
public:
	D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd);
	virtual ~D3D11Renderer();
	static D3D11Renderer* Instance() { return _instance; }
	bool Init();
	void Draw();

private:
	

private:
	static D3D11Renderer* _instance;

	HINSTANCE	m_hInst;
	HWND		m_hMainWnd;
	
	UINT		m_4xMsaaQuality;

	ID3D11Device* m_d3d11Device;
	ID3D11DeviceContext* m_d3d11DeviceContext;
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
	D3D_DRIVER_TYPE m_d3dDriverType;

	D3D11_VIEWPORT m_screenViewpot;

	int m_clientWidth;
	int m_clientHeight;
	bool m_enable4xMsaa;
};

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

	XMGLOBALCONST XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
}
#endif