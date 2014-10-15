#ifndef __D3D11_RENDERER__
#define __D3D11_RENDERER__

#include "stdafx.h"
#include "MeshRenderer.h"

class D3D11Renderer {
public:
	D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd);
	virtual ~D3D11Renderer();
	static D3D11Renderer* Instance() { return _instance; }
	bool Init();
	void Draw();
	ID3D11Device* GetD3DDevice() { return m_d3d11Device; }
	ID3D11DeviceContext* GetD3DContext() { return m_d3d11DeviceContext; }
	float GetAspectRatio() { return static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight); }

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


	MeshRenderer* mesh;
	MeshRenderer* mesh2;
};
#endif