#ifdef GRAPHICS_OPENGL

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

// somehow this are not in glew???
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute​;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;

namespace StenGine
{

void APIENTRY GLErrorCallback(GLenum source​, GLenum type​, GLuint id​, GLenum severity​, GLsizei length​, const GLchar* message​, const void* userParam​)
{
	if (GL_DEBUG_TYPE_OTHER == type​)
		return;

	cout << "message: " << message​ << endl;
	cout << "type: ";
	switch (type​) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << endl;

	cout << "id: " << id​ << endl;
	cout << "severity: ";
	switch (severity​) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	}
	cout << endl << endl;
}

Renderer* Renderer::_instance = nullptr;

class GLRenderer : public Renderer
{
public:
	GLRenderer(HINSTANCE hInstance, HWND hMainWnd)
		: m_hInst(hInstance)
		, m_hMainWnd(hMainWnd)
		, m_currentVao(0)
		, m_currentEffect(nullptr)
		, m_currentFbo(0)
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

		glDispatchCompute​ = (PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute");
		if (!glDispatchCompute​)
		{
			assert(false);
			return false;
		}

		glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)wglGetProcAddress("glBindImageTexture");
		if (!glBindImageTexture)
		{
			assert(false);
			return false;
		}

		wglSwapIntervalEXT(0);

#if !BUILD_RELEASE
		glDebugMessageCallback(GLErrorCallback, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

		glClearColor(0.2f, 0.2f, 0.2f, 0.f);
		glClearDepth(1.0f);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		
		/***************GBUFFER FB*********************/
		glCreateFramebuffers(1, &m_deferredGBuffers);
		GenerateColorTex(m_diffuseBufferTex);
		GenerateColorTex(m_normalBufferTex);
		GenerateColorTex(m_specularBufferTex);
		GenerateDepthTex(m_depthBufferTex);

		glNamedFramebufferTexture(m_deferredGBuffers, GL_DEPTH_ATTACHMENT, m_depthBufferTex, 0);
		glNamedFramebufferTexture(m_deferredGBuffers, GL_COLOR_ATTACHMENT0, m_normalBufferTex, 0);
		glNamedFramebufferTexture(m_deferredGBuffers, GL_COLOR_ATTACHMENT1, m_diffuseBufferTex, 0);
		glNamedFramebufferTexture(m_deferredGBuffers, GL_COLOR_ATTACHMENT2, m_specularBufferTex, 0);

		GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(m_deferredGBuffers, 3, draw_bufs);

		GLenum status = glCheckNamedFramebufferStatus(m_deferredGBuffers, GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != status) {
			assert(false);
			return false;
		}

		m_diffuseBufferTexHandle = glGetTextureHandleARB(m_diffuseBufferTex);
		m_normalBufferTexHandle = glGetTextureHandleARB(m_normalBufferTex);
		m_specularBufferTexHandle = glGetTextureHandleARB(m_specularBufferTex);
		m_depthBufferTexHandle = glGetTextureHandleARB(m_depthBufferTex);

		glMakeTextureHandleResidentARB(m_diffuseBufferTexHandle);
		glMakeTextureHandleResidentARB(m_normalBufferTexHandle);
		glMakeTextureHandleResidentARB(m_specularBufferTexHandle);
		glMakeTextureHandleResidentARB(m_depthBufferTexHandle);

		/****************deferred shading + ssao fb**********************/
		glCreateFramebuffers(1, &m_deferredShadingRT);

		GenerateColorTex(m_deferredShadingTex);
		GenerateColorTex(m_ssaoTex);
		GenerateDepthTex(m_deferredShadingDepthTex);

		glNamedFramebufferTexture(m_deferredShadingRT, GL_DEPTH_ATTACHMENT, m_deferredShadingDepthTex, 0);
		glNamedFramebufferTexture(m_deferredShadingRT, GL_COLOR_ATTACHMENT0, m_deferredShadingTex, 0);
		glNamedFramebufferTexture(m_deferredShadingRT, GL_COLOR_ATTACHMENT1, m_ssaoTex, 0);

		GLenum shading_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(m_deferredShadingRT, 2, shading_bufs);

		status = glCheckNamedFramebufferStatus(m_deferredShadingRT, GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != status) {
			assert(false);
			return false;
		}

		m_deferredShadingTexHandle = glGetTextureHandleARB(m_deferredShadingTex);
		m_ssaoTexHandle = glGetTextureHandleARB(m_ssaoTex);
		m_deferredShadingDepthTexHandle = glGetTextureHandleARB(m_deferredShadingDepthTex);

		glMakeTextureHandleResidentARB(m_deferredShadingTexHandle);
		glMakeTextureHandleResidentARB(m_ssaoTexHandle);
		glMakeTextureHandleResidentARB(m_deferredShadingDepthTexHandle);

		/******************************************************************/

		for (uint32_t i = 0; i < 4; ++i)
		{
			GenerateColorTex(m_computeOutput[i]);
			m_computeOutputHandle[i] = glGetTextureHandleARB(m_computeOutput[i]);
			glMakeTextureHandleResidentARB(m_computeOutputHandle[i]);
		}
		
		GLuint randVecTex = CreateGLTextureFromFile("Model/RandNorm.dds");
		glTextureParameteri(randVecTex, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTextureParameteri(randVecTex, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		m_randVecTexHandle = glGetTextureHandleARB(randVecTex);
		glMakeTextureHandleResidentARB(m_randVecTexHandle);

		DirectionalLight* dLight = new DirectionalLight();
		dLight->intensity = XMFLOAT4(1, 1, 1, 1);
		dLight->direction = MatrixHelper::NormalizeFloat3(XMFLOAT3(-0.5, -2, 1));
		dLight->castShadow = 1;

		LightManager::Instance()->m_dirLights.push_back(dLight);
		LightManager::Instance()->m_shadowMap = new ShadowMap(2048, 2048);

		m_SkyBox = std::unique_ptr<Skybox>(new Skybox(std::wstring(L"Model/sunsetcube1024.dds")));

		InitScreenQuad();
		InitDebugCoord();

		m_drawTopologyMap[PrimitiveTopology::POINTLIST] = GL_POINTS;
		m_drawTopologyMap[PrimitiveTopology::LINELIST] = GL_LINES;
		m_drawTopologyMap[PrimitiveTopology::TRIANGLELIST] = GL_TRIANGLES;
		m_drawTopologyMap[PrimitiveTopology::CONTROL_POINT_3_PATCHLIST] = GL_PATCHES;
		m_drawTopologyMap[PrimitiveTopology::CONTROL_POINT_4_PATCHLIST] = GL_PATCHES;

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

	void ExecuteCmdList()
	{
		for (auto &cmd : m_drawList)
		{
			if (cmd.flags & CmdFlag::BIND_FB)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)cmd.framebuffer);
			}

			if (cmd.flags & CmdFlag::SET_SS)
			{
				if (cmd.scissorState.scissorTestEnabled)
				{
					glEnable(GL_SCISSOR_TEST);
					glScissor(cmd.scissorState.x, cmd.scissorState.y, cmd.scissorState.width, cmd.scissorState.height);
				}
				else
				{
					glDisable(GL_SCISSOR_TEST);
				}
			}

			if (cmd.flags & CmdFlag::CLEAR_COLOR)
			{
				glClear(GL_COLOR_BUFFER_BIT);
			}

			if (cmd.flags & CmdFlag::CLEAR_DEPTH)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
			}

			if (cmd.flags & CmdFlag::SET_VP)
			{
				glViewport(
					(GLint)cmd.viewport.TopLeftX, 
					(GLint)cmd.viewport.TopLeftY, 
					(GLsizei)cmd.viewport.Width,
					(GLsizei)cmd.viewport.Height
				);
			}

			if (cmd.flags & CmdFlag::SET_BS)
			{
				if (cmd.blendState.blendEnable)
				{
					glEnable(GL_BLEND);

					static const uint32_t convertBlendFunc[] =
					{
						0,
						GL_FUNC_ADD,
						GL_FUNC_SUBTRACT,
						GL_FUNC_REVERSE_SUBTRACT,
						GL_MIN,
						GL_MAX,
					};

					glBlendEquationi(cmd.blendState.index, convertBlendFunc[(uint32_t)cmd.blendState.blendOpColor]);

					static const uint32_t convertBlend[] =
					{
						0,
						GL_ZERO,
						GL_ONE,
						GL_SRC_COLOR,
						GL_ONE_MINUS_SRC_COLOR,
						GL_SRC_ALPHA,
						GL_ONE_MINUS_SRC_ALPHA,
						GL_DST_ALPHA,
						GL_ONE_MINUS_DST_ALPHA,
						GL_DST_COLOR,
						GL_ONE_MINUS_DST_COLOR,
						GL_SRC_ALPHA_SATURATE,
					};

					glBlendFunci(cmd.blendState.index, convertBlend[(uint32_t)cmd.blendState.srcBlend], convertBlend[(uint32_t)cmd.blendState.destBlend]);
				}
				else
				{
					glDisable(GL_BLEND);
				}
			}

			if (cmd.flags & CmdFlag::SET_DS)
			{
				if (cmd.depthState.depthCompEnable)
				{
					glEnable(GL_DEPTH_TEST);

					static const uint32_t convertDepthFunc[] = 
					{
						0,
						GL_NEVER,
						GL_LESS,
						GL_EQUAL,
						GL_LEQUAL,
						GL_GREATER,
						GL_NOTEQUAL,
						GL_GEQUAL,
						GL_ALWAYS,
					};

					glDepthFunc(convertDepthFunc[(uint32_t)cmd.depthState.depthFunc]);
				}
				else
				{
					glDisable(GL_DEPTH_TEST);
				}

				if (cmd.depthState.depthWriteEnable)
				{
					glDepthMask(GL_TRUE);
				}
				else
				{
					glDepthMask(GL_FALSE);
				}
			}

			if (cmd.flags & CmdFlag::SET_CS)
			{
				if (cmd.rasterizerState.cullFaceEnabled)
				{
					glEnable(GL_CULL_FACE);
					static const uint32_t convertCull[] =
					{
						0,
						GL_CW,
						GL_CCW,
					};

					static const uint32_t convertType[] =
					{
						0,
						GL_FRONT,
						GL_BACK,
					};

					glFrontFace(convertCull[(uint32_t)cmd.rasterizerState.frontFace]);
					glCullFace(convertType[(uint32_t)cmd.rasterizerState.cullType]);
				}
				else
				{
					glDisable(GL_CULL_FACE);
				}
			}

			if (cmd.flags & CmdFlag::DRAW || cmd.flags & CmdFlag::COMPUTE)
			{
				cmd.effect->SetShader();

				for (auto &cbuffer : cmd.cbuffers)
				{
					cbuffer.Bind();
				}

				if (cmd.flags & CmdFlag::DRAW)
				{
					if (cmd.inputLayout)
						glBindVertexArray((GLuint)cmd.inputLayout);

					if (cmd.vertexBuffer)
						glBindVertexBuffer(0, (GLuint)cmd.vertexBuffer, cmd.vertexOffset, cmd.vertexStride);

					if (cmd.type == PrimitiveTopology::CONTROL_POINT_3_PATCHLIST)
					{
						glPatchParameteri(GL_PATCH_VERTICES, 3);
					}
					else if (cmd.type == PrimitiveTopology::CONTROL_POINT_4_PATCHLIST)
					{
						glPatchParameteri(GL_PATCH_VERTICES, 4);
					}

					if (cmd.drawType == DrawType::INDEXED)
					{
						if (cmd.indexBuffer)
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)cmd.indexBuffer);

						glDrawElements(
							m_drawTopologyMap[cmd.type],
							cmd.elementCount,
							GL_UNSIGNED_INT,
							cmd.offset
						);
					}
					else if (cmd.drawType == DrawType::ARRAY)
					{
						glDrawArrays(m_drawTopologyMap[cmd.type], (GLint)cmd.offset, cmd.elementCount);
					}
				}
				else if (cmd.flags & CmdFlag::COMPUTE)
				{
					cmd.uavs.Bind();
					glDispatchCompute​(cmd.threadGroupX, cmd.threadGroupY, cmd.threadGroupZ);
				}
			}
		}

		m_drawList.clear();
	}

	void Draw() override {
		EnterFrame();
		
		DrawShadowMap();
		DrawGBuffer();
		DrawDeferredShading();
		m_SkyBox->Draw();
		DrawBlurSSAOAndCombine();
		// TODO put every graphics call into cmdlist
		
		//DrawGodRay();
		DrawDebug();
		
		// TEST
		ImGui::NewFrame();
		//ImGui::ShowTestWindow();
		GameObjectManager::Instance()->DrawMenu();
		ImGui::Render();
		
		
		EndFrame();
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
		return m_SkyBox.get();
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

	RenderTarget GetGbuffer() override {
		return m_deferredGBuffers;
	}

	void EnterFrame()
	{
		// reset state
		DrawCmd cmd;
		cmd.flags = CmdFlag::SET_VP | CmdFlag::SET_BS | CmdFlag::SET_CS | CmdFlag::SET_SS | CmdFlag::SET_DS | CmdFlag::CLEAR_COLOR /*| CmdFlag::CLEAR_DEPTH*/;
		cmd.viewport.TopLeftX = 0;
		cmd.viewport.TopLeftY = 0;
		cmd.viewport.Width = m_clientWidth;
		cmd.viewport.Height = m_clientHeight;

		AddDeferredDrawCmd(cmd);
	}

	void EndFrame()
	{
		ExecuteCmdList();

		SwapBuffers(m_deviceContext);
	}

	void DrawShadowMap() override
	{
		LightManager::Instance()->m_shadowMap->UpdateShadowMatrix();

		uint32_t width, height;
		LightManager::Instance()->m_shadowMap->GetDimension(width, height);

		DrawCmd shadowcmd;

		shadowcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH;
		shadowcmd.framebuffer = (GLuint)LightManager::Instance()->m_shadowMap->GetRenderTarget();
		shadowcmd.viewport = { 0, 0, (float)width, (float)height, 0, 1 };

		m_drawList.push_back(std::move(shadowcmd));

		for (auto &gatherShadowDrawCall : m_shadowDrawHandler)
		{
			gatherShadowDrawCall();
		}
	}

	void DrawGBuffer() override 
	{
		DrawCmd drawcmd;

		drawcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH;
		drawcmd.framebuffer = (GLuint)m_deferredGBuffers;
		drawcmd.viewport = { 0.f, 0.f, (float)m_clientWidth, (float)m_clientHeight, 0.f, 1.f };

		m_drawList.push_back(std::move(drawcmd));

		for (auto &gatherDrawCall : m_drawHandler)
		{
			gatherDrawCall();
		}
	}

	void DrawDeferredShading() override {
		DrawCmd cmd;
		DeferredShadingPassEffect* effect = EffectsManager::Instance()->m_deferredShadingPassEffect.get();

		ConstantBuffer cbuffer0(0, sizeof(DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER), (void*)effect->m_perFrameCB);
		DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER* perFrameData = (DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();

		perFrameData->NormalGMap = m_normalBufferTexHandle;
		perFrameData->DiffuseGMap = m_diffuseBufferTexHandle;//LightManager::Instance()->m_shadowMap->GetDepthTex();//
		perFrameData->SpecularGMap = m_specularBufferTexHandle;
		perFrameData->DepthGMap = m_depthBufferTexHandle;
		perFrameData->RandVectMap = m_randVecTexHandle;

		XMMATRIX &viewMat = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
		XMMATRIX viewInvTranspose = MatrixHelper::InverseTranspose(viewMat);

		perFrameData->gDirLight = *LightManager::Instance()->m_dirLights[0];
		XMStoreFloat3(&perFrameData->gDirLight.direction, XMVector3Transform(XMLoadFloat3(&perFrameData->gDirLight.direction), viewInvTranspose));

		XMMATRIX &projMat = CameraManager::Instance()->GetActiveCamera()->GetProjMatrix();
		XMVECTOR det = XMMatrixDeterminant(projMat);
		perFrameData->gProj = projMat;
		perFrameData->gProjInv = XMMatrixInverse(&det, projMat);

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = (void*)m_screenQuadVAO;
		cmd.vertexBuffer = 0; // don't bind if 0
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = m_deferredShadingRT;
		cmd.offset = (void*)(0);
		cmd.effect = effect;
		cmd.elementCount = 6;
		cmd.cbuffers.push_back(std::move(cbuffer0));

		m_drawList.push_back(std::move(cmd));
	}

	void DrawBlurSSAOAndCombine() override {
		doCSBlur(m_ssaoTex, 0);

		// ------ Screen Quad -------//
		BlurEffect* blurEffect = EffectsManager::Instance()->m_blurEffect.get();

		DrawCmd cmd;

		// TODO
		//m_d3d11DeviceContext->PSSetSamplers(0, 1, samplerState);

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = (void*)m_screenQuadVAO;
		cmd.vertexBuffer = 0; // don't bind if 0
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = 0;
		cmd.offset = (void*)(0);
		cmd.effect = blurEffect;
		cmd.elementCount = 6;

		ConstantBuffer cbuffer0(0, sizeof(BlurEffect::SETTING_CONSTANT_BUFFER), (void*)blurEffect->m_settingCB);
		BlurEffect::SETTING_CONSTANT_BUFFER* settingData = (BlurEffect::SETTING_CONSTANT_BUFFER*)cbuffer0.GetBuffer();

		settingData->ScreenMap = m_deferredShadingTexHandle;
		settingData->SSAOMap = m_computeOutputHandle[1];
		settingData->DepthMap = m_depthBufferTexHandle;
		//settingData->BloomMap = NOT_USED;
		cmd.cbuffers.push_back(std::move(cbuffer0));

		AddDeferredDrawCmd(cmd);
	}

	void doCSBlur(GLuint blurImgSRV, int uavSlotIdx) {
		// vblur
		DrawCmd cmdV;

		VBlurEffect* vBlurEffect = EffectsManager::Instance()->m_vblurEffect.get();
		UINT numGroupsX = (UINT)ceilf(m_clientWidth / 256.0f);

		cmdV.effect = vBlurEffect;
		cmdV.flags = CmdFlag::COMPUTE;
		cmdV.threadGroupX = numGroupsX;
		cmdV.threadGroupY = m_clientHeight;
		cmdV.threadGroupZ = 1;
		cmdV.uavs.AddUAV(blurImgSRV, 0);
		cmdV.uavs.AddUAV(m_computeOutput[uavSlotIdx], 1);

		AddDeferredDrawCmd(std::move(cmdV));

		// hblur
		DrawCmd cmdH;
		
		HBlurEffect* hBlurEffect = EffectsManager::Instance()->m_hblurEffect.get();
		UINT numGroupsY = (UINT)ceilf(m_clientHeight / 256.0f);
		
		cmdH.effect = hBlurEffect;
		cmdH.flags = CmdFlag::COMPUTE;
		cmdH.threadGroupX = m_clientWidth;
		cmdH.threadGroupY = numGroupsY;
		cmdH.threadGroupZ = 1;
		cmdH.uavs.AddUAV(m_computeOutput[uavSlotIdx], 0);
		cmdH.uavs.AddUAV(m_computeOutput[uavSlotIdx + 1], 1);
		
		AddDeferredDrawCmd(std::move(cmdH));
	}

	void DrawGodRay() override {
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(m_screenQuadVAO);

		GodRayEffect* godRayFX = EffectsManager::Instance()->m_godrayEffect.get();

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
		DebugLineEffect* debugLineFX = EffectsManager::Instance()->m_debugLineEffect.get();

		DrawCmd cmd;

		cmd.flags = /*CmdFlag::BIND_FB |*/ CmdFlag::SET_DS | CmdFlag::DRAW;

		cmd.depthState.depthWriteEnable = false;
		cmd.effect = debugLineFX;
		cmd.inputLayout = (void*)m_debugCoordVAO;

		ConstantBuffer cbuffer0(0, sizeof(DebugLineEffect::PEROBJ_CONSTANT_BUFFER), (void*)debugLineFX->m_perObjectCB);
		DebugLineEffect::PEROBJ_CONSTANT_BUFFER* perObjectData = (DebugLineEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
		perObjectData->ViewProj = CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();

		cmd.cbuffers.push_back(std::move(cbuffer0));
		cmd.type = PrimitiveTopology::LINELIST;
		cmd.drawType = DrawType::ARRAY;
		cmd.framebuffer = 0;

		cmd.offset = (void*)6;
		cmd.elementCount = 44;

		DrawCmd cmd2;

		cmd2.effect = debugLineFX;
		cmd2.flags = CmdFlag::DRAW;
		cmd2.offset = 0;
		cmd2.elementCount = 6;
		cmd2.type = PrimitiveTopology::LINELIST;
		cmd2.drawType = DrawType::ARRAY;

		AddDeferredDrawCmd(cmd);
		AddDeferredDrawCmd(cmd2);
	}

	GLRenderer::~GLRenderer()
	{
		glMakeTextureHandleNonResidentARB(m_diffuseBufferTexHandle);
		glMakeTextureHandleNonResidentARB(m_normalBufferTexHandle);
		glMakeTextureHandleNonResidentARB(m_specularBufferTexHandle);
		glMakeTextureHandleNonResidentARB(m_depthBufferTexHandle);
	}

	void GenerateColorTex(GLuint &bufferTex) {
		glCreateTextures(GL_TEXTURE_2D, 1, &bufferTex);
		glTextureStorage2D(bufferTex, 1, GL_RGBA16F, m_clientWidth, m_clientHeight);

		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(bufferTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(bufferTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	void GenerateDepthTex(GLuint &bufferTex) {
		glCreateTextures(GL_TEXTURE_2D, 1, &bufferTex);
		glTextureStorage2D(bufferTex, 1, GL_DEPTH_COMPONENT32, m_clientWidth, m_clientHeight);

		glTextureParameteri(bufferTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(bufferTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void AddDeferredDrawCmd(DrawCmd &cmd)
	{
		//m_deferredDrawList.push_back(std::move(cmd));
		m_drawList.push_back(std::move(cmd));
	}

	void AddShadowDrawCmd(DrawCmd &cmd)
	{
		//m_shadowMapDrawList.push_back(std::move(cmd));
		m_drawList.push_back(std::move(cmd));
	}

	void AddDraw(DrawEventHandler handler)
	{
		m_drawHandler.push_back(handler);
	}

	void AddShadowDraw(DrawEventHandler handler)
	{
		m_shadowDrawHandler.push_back(handler);
	}

private:
	int m_clientWidth;
	int m_clientHeight;
	bool m_enable4xMsaa;
	std::unique_ptr<Skybox> m_SkyBox;

	HINSTANCE	m_hInst;
	HWND		m_hMainWnd;
	HDC			m_deviceContext;
	HGLRC		m_renderingContext;

	GLuint m_deferredGBuffers;

	GLuint m_diffuseBufferTex;
	GLuint m_normalBufferTex;
	GLuint m_specularBufferTex;
	GLuint m_depthBufferTex;

	GLuint m_deferredShadingRT;;

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
		glNamedBufferStorage(screenQuadVertexVBO, quadVertexBuffer.size() * sizeof(XMFLOAT4), &quadVertexBuffer[0], 0);

		glCreateBuffers(1, &screenQuadUVVBO);
		glNamedBufferStorage(screenQuadUVVBO, quadUvVertexBuffer.size() * sizeof(XMFLOAT2), &quadUvVertexBuffer[0], 0);

		glCreateVertexArrays(1, &m_screenQuadVAO);

		glEnableVertexArrayAttrib(m_screenQuadVAO, 0);
		glEnableVertexArrayAttrib(m_screenQuadVAO, 1);

		glVertexArrayVertexBuffer(m_screenQuadVAO, 0, screenQuadVertexVBO, 0, sizeof(XMFLOAT4));
		glVertexArrayVertexBuffer(m_screenQuadVAO, 1, screenQuadUVVBO, 0, sizeof(XMFLOAT2));

		glVertexArrayAttribFormat(m_screenQuadVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_screenQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, 0);

		glVertexArrayAttribBinding(m_screenQuadVAO, 0, 0);
		glVertexArrayAttribBinding(m_screenQuadVAO, 1, 1);
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
		glNamedBufferStorage(debugDrawVertexVBO, coordVertexBuffer.size() * sizeof(XMFLOAT3), &coordVertexBuffer[0], 0);

		glCreateVertexArrays(1, &m_debugCoordVAO);

		glEnableVertexArrayAttrib(m_debugCoordVAO, 0);
		glVertexArrayAttribFormat(m_debugCoordVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayVertexBuffer(m_debugCoordVAO, 0, debugDrawVertexVBO, 0, sizeof(XMFLOAT3));
		glVertexArrayAttribBinding(m_debugCoordVAO, 0, 0);
	}
};

Renderer* Renderer::Create(HINSTANCE hInstance, HWND hMainWnd)
{
	GLRenderer* renderer = new GLRenderer(hInstance, hMainWnd);
	_instance = static_cast<Renderer*>(renderer);
	return _instance;
}

}

#endif