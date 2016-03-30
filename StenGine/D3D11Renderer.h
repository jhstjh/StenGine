#ifndef __D3D11_RENDERER__
#define __D3D11_RENDERER__

#include "stdafx.h"
#include "Skybox.h"
#include "RendererBase.h"



class D3D11Renderer: public Renderer {
public:
	D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd);
	virtual ~D3D11Renderer();
	static D3D11Renderer* Instance() { return _instance; }
	virtual bool Init();
	virtual void Draw();

	void DrawGBuffer();
	void DrawDeferredShading();
	void DrawBlurSSAOAndCombine();
	void DrawDebug();
	void DrawGodRay();

	ID3D11ShaderResourceView* doCSBlur(ID3D11ShaderResourceView* blurImgSRV, int uavSlotIdx = 0);

	ID3D11Device* GetD3DDevice() { return m_d3d11Device; }
	ID3D11DeviceContext* GetD3DContext() { return m_d3d11DeviceContext; }

	ID3D11RasterizerState* m_depthRS;
	Skybox* m_SkyBox;

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
	D3D11_VIEWPORT m_screenSuperSampleViewpot;
	ID3D11RasterizerState* m_wireFrameRS;
	ID3D11SamplerState* m_samplerState;
	ID3D11SamplerState* m_heightMapSamplerState;
	ID3D11SamplerState* m_shadowSamplerState;
	ID3D11SamplerState* m_borderSamplerState;

	ID3D11DepthStencilState* m_noZWriteDSState;
	ID3D11BlendState* m_additiveAlphaAddBS;

#pragma region DEDERRED_RENDER
	ID3D11RenderTargetView* m_diffuseBufferRTV;
	ID3D11RenderTargetView* m_normalBufferRTV;
	ID3D11RenderTargetView* m_specularBufferRTV;
	ID3D11RenderTargetView* m_positionBufferRTV;
	ID3D11RenderTargetView* m_edgeBufferRTV;
	ID3D11RenderTargetView* m_SSAORTV;
	ID3D11RenderTargetView* m_SSAORTV2;
	ID3D11RenderTargetView* m_deferredShadingRTV;

	ID3D11ShaderResourceView* m_randVecTexSRV;

	ID3D11ShaderResourceView* m_diffuseBufferSRV;
	ID3D11ShaderResourceView* m_normalBufferSRV;
	ID3D11ShaderResourceView* m_specularBufferSRV;
	ID3D11ShaderResourceView* m_positionBufferSRV;
	ID3D11ShaderResourceView* m_edgeBufferSRV;
	ID3D11ShaderResourceView* m_SSAOSRV;
	ID3D11ShaderResourceView* m_SSAOSRV2;
	ID3D11ShaderResourceView* m_deferredShadingSRV;

	ID3D11DepthStencilView* m_deferredRenderDepthStencilView;
	ID3D11ShaderResourceView* m_deferredRenderShaderResourceView;

#pragma endregion

	bool m_enable4xMsaa;

	ID3D11Buffer* m_gridCoordIndexBufferGPU;
	ID3D11Buffer* m_gridCoordVertexBufferGPU;
};
#endif