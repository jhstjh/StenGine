#pragma once

#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Color.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/Terrain.h"
#include "Scene/LightManager.h"
#include "Scene/CameraManager.h"
#include "Math/MathHelper.h"
#include "Engine/EventSystem.h"

#include <unordered_map>
#include <map>

#include "Scene/GameObjectManager.h"
#include "imgui.h"

#pragma warning(disable: 4267 4244 4311 4302)

namespace StenGine
{

class D3D11Renderer : public Renderer
{
public:
	D3D11Renderer(HINSTANCE hInstance, HWND hMainWnd);
	virtual ~D3D11Renderer();
	virtual void Release();
	virtual bool Init(int32_t width, int32_t height, CreateWindowCallback createWindow);
	virtual void Draw();
	virtual float GetAspectRatio();
	virtual int GetScreenWidth();
	virtual int GetScreenHeight();
	virtual Skybox* GetSkyBox();
	virtual void* GetDepthRS();
	void ExecuteCmdList();
	virtual void DrawShadowMap();
	virtual void DrawGBuffer();
	virtual void DrawDeferredShading();
	virtual void DrawBlurSSAOAndCombine();
	virtual void DrawGodRay();
	virtual void DrawDebug();
	void doCSBlur(ID3D11ShaderResourceView* blurImgSRV, int uavSlotIdx);
	ID3D11BlendState* GetBlendState(BlendState& blendState);
	ID3D11DepthStencilState* GetDepthState(DepthState& depthState);
	ID3D11RasterizerState* GetRasterizerState(RasterizerState & rasterizerState);
	virtual void* GetDevice();
	virtual void* GetDeviceContext();
	void UpdateTitle(const char* str);
	virtual void AddDeferredDrawCmd(DrawCmd &cmd);
	virtual void AddShadowDrawCmd(DrawCmd &cmd);
	virtual RenderTarget &GetGbuffer();
	virtual void AddDraw(DrawEventHandler handler);
	virtual void AddShadowDraw(DrawEventHandler handler);

private:
	int m_clientWidth;
	int m_clientHeight;
	std::unique_ptr<Skybox> m_SkyBox;

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

	ID3D11ShaderResourceView* m_outputShaderResources[4];
	ID3D11UnorderedAccessView* m_unorderedAccessViews[4];

	bool m_enable4xMsaa;

	ID3D11Buffer* m_gridCoordIndexBufferGPU;
	ID3D11Buffer* m_gridCoordVertexBufferGPU;

	ID3D11RasterizerState* m_depthRS;

	std::vector<DrawCmd> m_drawList;

	std::vector<DrawEventHandler> m_drawHandler;
	std::vector<DrawEventHandler> m_shadowDrawHandler;

	std::unordered_map<PrimitiveTopology, D3D_PRIMITIVE_TOPOLOGY> m_drawTopologyMap;
	std::unordered_map<BlendState, ID3D11BlendState*> m_blendStateMap;
	std::unordered_map<DepthState, ID3D11DepthStencilState*> m_depthStateMap;
	std::unordered_map<RasterizerState, ID3D11RasterizerState*> m_rasterizerStateMap;

	RenderTarget m_GBuffer;
	RenderTarget m_deferredShadingRT;
	RenderTarget m_defaultRTNoDepth;
	RenderTarget m_defaultRTWithDepth;
	RenderTarget m_debugRT;
};



}
