#pragma once

#include "glew.h"
#include "wglew.h"

#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Mesh/MeshRenderer.h"
#include "Scene/LightManager.h"
#include "Math/MathHelper.h"
#include "Scene/CameraManager.h"
#include "Graphics/Color.h"
#include "Scene/LightManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Graphics/OpenGL/GLImageLoader.h"
#include "Engine/EventSystem.h"
#include "imgui.h"
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>

//TEST
#include "Scene/GameObjectManager.h"

using namespace std;

#pragma warning(disable: 4244) // conversion from 'int64_t' to 'GLsizei', possible loss of data
#pragma warning(disable: 4312 4311 4302) // 'type cast': conversion from 'GLuint' to 'void *' of greater size

namespace StenGine
{

class GLRenderer : public Renderer
{
public:
	GLRenderer(HINSTANCE hInstance, HWND hMainWnd);

	virtual void Release() override;

	virtual bool Init(int32_t width, int32_t height, CreateWindowCallback createWindow) override;

	bool initializeExtensions();

	bool initializeOpenGL();

	void ExecuteCmdList();

	virtual void Draw() override;

	virtual float GetAspectRatio() override;

	virtual	int GetScreenWidth() override;

	virtual int GetScreenHeight() override;

	virtual Skybox* GetSkyBox() override;

	virtual void* GetDevice() override;

	virtual void* GetDeviceContext();

	void* GetDepthRS() override;

	void UpdateTitle(const char* str) override;

	RenderTarget &GetGbuffer() override;

	void EnterFrame();

	void EndFrame();

	virtual void DrawShadowMap() override;

	virtual void DrawGBuffer() override;

	virtual	void DrawDeferredShading() override;

	virtual void DrawBlurSSAOAndCombine() override;

	void doCSBlur(GLuint blurImgSRV, int uavSlotIdx);

	virtual void DrawDebug() override;

	virtual GLRenderer::~GLRenderer();

	void GenerateColorTex(GLuint &bufferTex);

	void GenerateDepthTex(GLuint &bufferTex);

	virtual void AddDeferredDrawCmd(DrawCmd &cmd) override;

	virtual void AddShadowDrawCmd(DrawCmd &cmd) override;

	virtual void AddDraw(DrawEventHandler handler) override;

	virtual void AddShadowDraw(DrawEventHandler handler) override;

private:
	int m_clientWidth;
	int m_clientHeight;
	bool m_enable4xMsaa;
	std::unique_ptr<Skybox> m_SkyBox;

	HINSTANCE	m_hInst;
	HWND		m_hMainWnd;
	HDC			m_deviceContext;
	HGLRC		m_renderingContext;

	RenderTarget m_deferredGBuffers;

	GLuint m_diffuseBufferTex;
	GLuint m_normalBufferTex;
	GLuint m_specularBufferTex;
	GLuint m_depthBufferTex;

	RenderTarget m_deferredShadingRT;;
	RenderTarget m_defaultRT;;

	GLuint m_ssaoTex;
	GLuint m_deferredShadingTex;
	GLuint m_deferredShadingDepthTex;

	GLuint m_computeOutput[4];
	uint64_t m_computeOutputHandle[4];

	uint64_t m_randVecTexHandle;

	uint64_t m_diffuseBufferTexHandle;
	uint64_t m_normalBufferTexHandle;
	uint64_t m_specularBufferTexHandle;
	uint64_t m_depthBufferTexHandle;

	uint64_t m_ssaoTexHandle;
	uint64_t m_deferredShadingTexHandle;
	uint64_t m_deferredShadingDepthTexHandle;

	GLuint m_debugCoordVAO;
	GLuint m_screenQuadVAO;

	std::vector<DrawCmd> m_drawList;

	uint64_t m_currentVao;
	Effect* m_currentEffect;
	uint64_t m_currentFbo;

	std::vector<DrawEventHandler> m_drawHandler;
	std::vector<DrawEventHandler> m_shadowDrawHandler;

	std::unordered_map<PrimitiveTopology, GLenum> m_drawTopologyMap;

	void InitScreenQuad();

	void InitDebugCoord();
};


}
