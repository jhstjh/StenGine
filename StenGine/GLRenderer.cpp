#ifdef GRAPHICS_OPENGL

#include "GL/glew.h"
#include "GL/wglew.h"

#include "RendererBase.h"
#include "D3DIncludes.h"
#include "EffectsManager.h"
#include "MeshRenderer.h"
#include "LightManager.h"
#include "MathHelper.h"
#include "CameraManager.h"
#include "Color.h"
#include "LightManager.h"
#include "ShadowMap.h"
#include "Skybox.h"
#include <vector>
#include <memory>


void APIENTRY GLErrorCallback(GLenum source​, GLenum type​, GLuint id​, GLenum severity​, GLsizei length​, const GLchar* message​, const void* userParam​)
{
	if (GL_DEBUG_SEVERITY_HIGH == severity​)
	{
		printf("%s\n", message​);
	}
}

Renderer* Renderer::_instance = nullptr;

class GLRenderer : public Renderer
{
public:
	GLRenderer(HINSTANCE hInstance, HWND hMainWnd) 
		: m_hInst(hInstance)
		, m_hMainWnd(hMainWnd)
	{
		_instance = this;
	}
	
	void Release() override {
		_instance = nullptr;
		delete this;
	}

	bool Init(int32_t width, int32_t height, CreateWindowCallback createWindow) override {
		m_clientWidth = width;
		m_clientHeight = height;
		
		if (!createWindow(0, 0, m_hInst, m_hMainWnd))
		{
			return false;
		}

		if (!initializeExtensions())
		{
			return false;
		}

		DestroyWindow(m_hMainWnd);

		if (!createWindow(width, height, m_hInst, m_hMainWnd))
		{
			return false;
		}

		SetWindowText(m_hMainWnd, L"StenGine");
		ShowWindow(m_hMainWnd, SW_SHOW);

		SetFocus(m_hMainWnd);

		if (!initializeOpenGL())
		{
			return false;
		}

		wglSwapIntervalEXT(0);

		glDebugMessageCallback(GLErrorCallback, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

		glClearColor(0.2f, 0.2f, 0.2f, 0.f);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST); // enable depth-testing
		glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
		glEnable(GL_CULL_FACE); // cull face
		glCullFace(GL_BACK); // cull back face
		glFrontFace(GL_CW); // GL_CCW for counter clock-wise
		glDepthMask(GL_TRUE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glViewport(0, 0, m_clientWidth, m_clientHeight);
		glGenFramebuffers(1, &m_deferredGBuffers);
		GenerateColorTex(m_diffuseBufferTex);
		GenerateColorTex(m_normalBufferTex);
		GenerateColorTex(m_specularBufferTex);
		GenerateDepthTex(m_depthBufferTex);

		glBindFramebuffer(GL_FRAMEBUFFER, m_deferredGBuffers);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBufferTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_normalBufferTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_diffuseBufferTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_specularBufferTex, 0);

		GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, draw_bufs);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != status) {
			assert(false);
			return false;
		}
		
		DirectionalLight* dLight = new DirectionalLight();
		dLight->intensity = XMFLOAT4(1, 1, 1, 1);
		dLight->direction = MatrixHelper::NormalizeFloat3(XMFLOAT3(-0.5, -2, 1));
		dLight->castShadow = 1;

		LightManager::Instance()->m_dirLights.push_back(dLight);
		LightManager::Instance()->m_shadowMap = new ShadowMap(1024, 1024);

		m_SkyBox = new Skybox(std::wstring(L"Model/sunsetcube1024.dds"));

		InitScreenQuad();

		InitDebugCoord();

		return true;
	}

	bool initializeExtensions()
	{
		HDC deviceContext;
		PIXELFORMATDESCRIPTOR pixelFormat;
		int error;
		HGLRC renderContext;
		//bool result;

		deviceContext = GetDC(m_hMainWnd);
		if (!deviceContext)
		{
			return false;
		}

		error = SetPixelFormat(deviceContext, 1, &pixelFormat);
		if (error != 1)
		{
			return false;
		}

		renderContext = wglCreateContext(deviceContext);
		if (!renderContext)
		{
			return false;
		}

		error = wglMakeCurrent(deviceContext, renderContext);
		if (error != 1)
		{
			return false;
		}

		GLenum glewerr = glewInit();
		if (GLEW_OK != glewerr)
		{
			OutputDebugStringA("GLEW is not initialized!\n");
			return false;
		}

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(renderContext);
		renderContext = nullptr;

		ReleaseDC(m_hMainWnd, deviceContext);
		deviceContext = 0;

		return true;
	}

	bool initializeOpenGL()
	{
		int pixelFormat[1];
		unsigned int formatCount;
		int result;
		PIXELFORMATDESCRIPTOR pixelFormatDescriptor;

		m_deviceContext = GetDC(m_hMainWnd);
		if (!m_deviceContext)
		{
			return false;
		}

		int attributeListInt[] = {
			WGL_SUPPORT_OPENGL_ARB, TRUE,
			WGL_DRAW_TO_WINDOW_ARB, TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_DOUBLE_BUFFER_ARB, TRUE,
			WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			0
		};

		result = wglChoosePixelFormatARB(m_deviceContext, attributeListInt, NULL, 1, pixelFormat, &formatCount);
		if (result != 1)
		{
			return false;
		}

		result = SetPixelFormat(m_deviceContext, pixelFormat[0], &pixelFormatDescriptor);
		if (result != 1)
		{
			return false;
		}

		// Set the 4.5 version of OpenGL
		int attributeList[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 5,
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			0
		};

		m_renderingContext = wglCreateContextAttribsARB(m_deviceContext, 0, attributeList);
		if (m_renderingContext == NULL)
		{
			return false;
		}

		wglMakeCurrent(m_deviceContext, m_renderingContext);

		return true;
	}

	void Draw() override {

		LightManager::Instance()->m_shadowMap->RenderShadowMap();

		DrawGBuffer();
		DrawDeferredShading();
		//DrawBlurSSAOAndCombine();
		//DrawGodRay();
		DrawDebug();

		SwapBuffers(m_deviceContext);
	}

	float GetAspectRatio() override {
		return static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight); 
	}

	int GetScreenWidth() override {
		return m_clientWidth; 
	}

	int GetScreenHeight() override {
		return m_clientHeight; 
	}

	virtual Skybox* GetSkyBox() override {
		return m_SkyBox;
	}

	virtual void* GetDevice() override {
		return nullptr;
	}

	virtual void* GetDeviceContext() override {
		return nullptr;
	}

	void* GetDepthRS() override {
		return nullptr;
	}

	void UpdateTitle(const char* str) override {
		SetWindowTextA(m_hMainWnd, str);
	}

	void DrawGBuffer() override {
		glViewport(0, 0, m_clientWidth, m_clientHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, m_deferredGBuffers);
		EffectsManager::Instance()->m_deferredGeometryPassEffect->SetShader();
		DeferredGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredGeometryPassEffect;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO: should separate perobj and perframe's updateconstantbuffer

		for (uint32_t iMesh = 0; iMesh < EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes.size(); iMesh++) {
			EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes[iMesh]->GatherDrawCall();
		}

		for (auto &cmd : m_deferredDrawList)
		{
			cmd.m_effect->SetShader();
			glBindVertexArray(cmd.m_vertexArrayObject);

			for (auto &tex : cmd.m_textures)
			{
				tex.Bind();
			}

			for (auto &cbuffer : cmd.m_cbuffers)
			{
				cbuffer.Bind();
			}

			glDrawElements(
				GL_TRIANGLES,
				cmd.m_elementCount,
				GL_UNSIGNED_INT,
				cmd.m_offset
			);
		}

		m_deferredDrawList.clear();
	}

	void DrawDeferredShading() override {
		DeferredShadingPassEffect* deferredShadingFX = EffectsManager::Instance()->m_deferredShadingPassEffect;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		deferredShadingFX->SetShader();
		glBindVertexArray(m_screenQuadVAO);

		deferredShadingFX->DiffuseGMap = m_diffuseBufferTex;//LightManager::Instance()->m_shadowMap->GetDepthTex();//
		deferredShadingFX->NormalGMap = m_normalBufferTex;
		deferredShadingFX->SpecularGMap = m_specularBufferTex;
		deferredShadingFX->DepthGMap = m_depthBufferTex;
		deferredShadingFX->BindShaderResource();

		XMMATRIX &viewMat = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
		XMMATRIX viewInvTranspose = MatrixHelper::InverseTranspose(viewMat);

		deferredShadingFX->m_perFrameUniformBuffer.gDirLight = *LightManager::Instance()->m_dirLights[0];
		XMStoreFloat3(&deferredShadingFX->m_perFrameUniformBuffer.gDirLight.direction, XMVector3Transform(XMLoadFloat3(&deferredShadingFX->m_perFrameUniformBuffer.gDirLight.direction), viewInvTranspose));

		XMMATRIX &projMat = CameraManager::Instance()->GetActiveCamera()->GetProjMatrix();
		XMVECTOR det = XMMatrixDeterminant(projMat);
		deferredShadingFX->m_perFrameUniformBuffer.gProj = projMat;
		deferredShadingFX->m_perFrameUniformBuffer.gProjInv = XMMatrixInverse(&det, projMat);
		deferredShadingFX->UpdateConstantBuffer();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		m_SkyBox->Draw();

		deferredShadingFX->UnSetShader();
		deferredShadingFX->UnBindShaderResource();
	}

	void DrawBlurSSAOAndCombine() override {

	}

	void DrawGodRay() override {
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(m_screenQuadVAO);

		GodRayEffect* godRayFX = EffectsManager::Instance()->m_godrayEffect;

		XMFLOAT3 lightDir = LightManager::Instance()->m_dirLights[0]->direction;
		XMVECTOR lightPos = -400 * XMLoadFloat3(&lightDir);
		XMVECTOR lightPosH = XMVector3Transform(lightPos, CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
		XMFLOAT4 lightPosHf;
		XMStoreFloat4(&lightPosHf, lightPosH);
		lightPosHf.x /= lightPosHf.w;
		lightPosHf.x = 0.5f + lightPosHf.x / 2;
		lightPosHf.y /= lightPosHf.w;
		lightPosHf.y = 0.5f + lightPosHf.y / 2;
		lightPosHf.z /= lightPosHf.w;

		godRayFX->OcclusionMap = m_normalBufferTex;
		godRayFX->m_perFrameUniformBuffer.gLightPosH = lightPosHf;
		godRayFX->SetShader();
		godRayFX->UpdateConstantBuffer();
		godRayFX->BindConstantBuffer();
		godRayFX->BindShaderResource();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		godRayFX->UnBindConstantBuffer();
		godRayFX->UnBindShaderResource();
		godRayFX->UnSetShader();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	void DrawDebug() override {
		glDepthMask(GL_FALSE);
		// copy depth into default depth buffer
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredGBuffers);
		glBlitFramebuffer(0, 0, m_clientWidth, m_clientHeight,
			0, 0, m_clientWidth, m_clientHeight,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DebugLineEffect* debugLineFX = EffectsManager::Instance()->m_debugLineEffect;
		debugLineFX->SetShader();
		glBindVertexArray(m_debugCoordVAO);
		debugLineFX->m_perObjUniformBuffer.ViewProj = CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();
		debugLineFX->UpdateConstantBuffer();
		glDrawArrays(GL_LINES, 6, 44);
		glDrawArrays(GL_LINES, 0, 6);
		debugLineFX->UnSetShader();
		debugLineFX->UnBindShaderResource();
		glDepthMask(GL_TRUE);
	}

	GLRenderer::~GLRenderer() {}

	void GenerateColorTex(GLuint &bufferTex) {
		glGenTextures(1, &bufferTex);
		glBindTexture(GL_TEXTURE_2D, bufferTex);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA16F,
			m_clientWidth,
			m_clientHeight,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			nullptr
			);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	void GenerateDepthTex(GLuint &bufferTex) {
		glGenTextures(1, &bufferTex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bufferTex);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT,
			m_clientWidth,
			m_clientHeight,
			0,
			GL_DEPTH_COMPONENT,
			GL_UNSIGNED_BYTE, NULL
			);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void AddDeferredDrawCmd(DrawCmd &cmd)
	{
		m_deferredDrawList.push_back(std::move(cmd));
	}

private:
	int m_clientWidth;
	int m_clientHeight;
	bool m_enable4xMsaa;
	Skybox* m_SkyBox;

	HINSTANCE	m_hInst;
	HWND		m_hMainWnd;
	HDC			m_deviceContext;
	HGLRC		m_renderingContext;

	GLuint m_deferredGBuffers;

	GLuint m_diffuseBufferTex;
	GLuint m_normalBufferTex;
	GLuint m_specularBufferTex;
	GLuint m_depthBufferTex;

	GLuint m_debugCoordVAO;
	GLuint m_screenQuadVAO;

	std::vector<DrawCmd> m_deferredDrawList;

	void InitScreenQuad()
	{
		// init screen quad vbo
		std::vector<XMFLOAT4> quadVertexBuffer = {
			XMFLOAT4(-1.0, -1.0, -1.0, 1.0),
			XMFLOAT4(-1.0, 1.0, -1.0, 1.0),
			XMFLOAT4(1.0, 1.0, -1.0, 1.0),
			XMFLOAT4(1.0, 1.0, -1.0, 1.0),
			XMFLOAT4(1.0, -1.0, -1.0, 1.0),
			XMFLOAT4(-1.0, -1.0, -1.0, 1.0),
		};

		std::vector<XMFLOAT2> quadUvVertexBuffer = {
			XMFLOAT2(0, 0),
			XMFLOAT2(0, 1),
			XMFLOAT2(1, 1),
			XMFLOAT2(1, 1),
			XMFLOAT2(1, 0),
			XMFLOAT2(0, 0),
		};

		GLuint screenQuadVertexVBO;
		GLuint screenQuadUVVBO;
		glCreateBuffers(1, &screenQuadVertexVBO);
		
		glBindBuffer(GL_ARRAY_BUFFER, screenQuadVertexVBO);
		glBufferData(GL_ARRAY_BUFFER, quadVertexBuffer.size() * sizeof(XMFLOAT4), &quadVertexBuffer[0], GL_STATIC_DRAW);

		glCreateBuffers(1, &screenQuadUVVBO);
		glBindBuffer(GL_ARRAY_BUFFER, screenQuadUVVBO);
		glBufferData(GL_ARRAY_BUFFER, quadUvVertexBuffer.size() * sizeof(XMFLOAT2), &quadUvVertexBuffer[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &m_screenQuadVAO);
		glBindVertexArray(m_screenQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, screenQuadVertexVBO);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, screenQuadUVVBO);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

	void InitDebugCoord()
	{
		// init grid and coord debug draw
		std::vector<XMFLOAT3> coordVertexBuffer = {
			XMFLOAT3(0, 0, 0),
			XMFLOAT3(5, 0, 0),
			XMFLOAT3(0, 0, 0),
			XMFLOAT3(0, 5, 0),
			XMFLOAT3(0, 0, 0),
			XMFLOAT3(0, 0, 5),
		};

		std::vector<UINT> coordIndexBuffer = { 0, 1, 2, 3, 4, 5 };

		int initIdx = 6;
		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.push_back(XMFLOAT3(-5.f, 0.f, -5.f + i));
			coordVertexBuffer.push_back(XMFLOAT3(5.f, 0.f, -5.f + i));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.push_back(XMFLOAT3(-5.f + i, 0.f, -5.f));
			coordVertexBuffer.push_back(XMFLOAT3(-5.f + i, 0.f, 5.f));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		GLuint debugDrawVertexVBO;
		glCreateBuffers(1, &debugDrawVertexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexVBO);
		glBufferData(GL_ARRAY_BUFFER, coordVertexBuffer.size() * sizeof(XMFLOAT3), &coordVertexBuffer[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &m_debugCoordVAO);
		glBindVertexArray(m_debugCoordVAO);
		glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
	}
};

Renderer* Renderer::Create(HINSTANCE hInstance, HWND hMainWnd)
{
	GLRenderer* renderer = new GLRenderer(hInstance, hMainWnd);
	_instance = static_cast<Renderer*>(renderer);
	return _instance;
}

#endif