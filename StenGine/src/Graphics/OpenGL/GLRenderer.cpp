#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <atomic>
#include <thread>

#include <glew.h>
#include <imgui.h>
#include <wglew.h>

#include "Engine/EventSystem.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Color.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Effect/Skybox.h"
#include "Graphics/D3D11/D3D11Buffer.h"
#include "Graphics/OpenGL/GLConstantBuffer.h"
#include "Graphics/OpenGL/GLImageLoader.h"
#include "Graphics/OpenGL/GLRenderTarget.h"
#include "Graphics/OpenGL/GLTexture.h"
#include "Graphics/OpenGL/GLUAVBinding.h"
#include "Math/MathHelper.h"
#include "Mesh/MeshRenderer.h"
#include "Scene/LightManager.h"
#include "Scene/CameraManager.h"
#include "Scene/SceneFileManager.h"
#include "Utility/Semaphore.h"

//TEST
#include "Scene/GameObjectManager.h"


using namespace std;

#pragma warning(disable: 4244) // conversion from 'int64_t' to 'GLsizei', possible loss of data
#pragma warning(disable: 4312 4311 4302) // 'type cast': conversion from 'GLuint' to 'void *' of greater size

extern "C"
{
	// somehow this are not in glew???
	PFNGLDISPATCHCOMPUTEPROC glDispatchCompute​;
}

namespace StenGine
{

static void APIENTRY GLErrorCallback(GLenum source​, GLenum type​, GLuint id​, GLenum severity​, GLsizei length​, const GLchar* message​, const void* userParam​)
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

class GLRenderer : public Renderer
{
public:

	GLRenderer(HINSTANCE hInstance, HWND hMainWnd, Semaphore &prepareDrawListSync, Semaphore &finishedDrawListSync)
		: mHInst(hInstance)
		, mHMainWnd(hMainWnd)
		, mCurrentVao(0)
		, mCurrentEffect(nullptr)
		, mCurrentFbo(0)
		, gPrepareDrawListSync(prepareDrawListSync)
		, gFinishedDrawListSync(finishedDrawListSync)
		, mEnableSSAO(true)
	{
		_instance = this;
	}

	void Release() final {
		_instance = nullptr;
		delete this;
	}

	bool Init(int32_t width, int32_t height, CreateWindowCallback createWindow) final {
		mClientWidth = width;
		mClientHeight = height;

		if (!createWindow(0, 0, mHInst, mHMainWnd))
		{
			return false;
		}

		if (!initializeExtensions())
		{
			return false;
		}

		DestroyWindow(mHMainWnd);

		if (!createWindow(width, height, mHInst, mHMainWnd))
		{
			return false;
		}

		SetWindowText(mHMainWnd, L"StenGine");
		ShowWindow(mHMainWnd, SW_SHOW);

		SetFocus(mHMainWnd);

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
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLErrorCallback, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
#endif

		glClearColor(0.2f, 0.2f, 0.2f, 0.f);
		glClearDepth(1.0f);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		mDefaultRT = CreateRenderTarget();
		mDefaultRT->Set(0);

		/***************GBUFFER FB*********************/
		GLuint gbuffer;
		glCreateFramebuffers(1, &gbuffer);
		GenerateColorTex(mDiffuseBufferTex);
		GenerateColorTex(mNormalBufferTex);
		GenerateColorTex(mSpecularBufferTex);
		GenerateDepthTex(mDepthBufferTex);

		glNamedFramebufferTexture(gbuffer, GL_DEPTH_ATTACHMENT, mDepthBufferTex, 0);
		glNamedFramebufferTexture(gbuffer, GL_COLOR_ATTACHMENT0, mNormalBufferTex, 0);
		glNamedFramebufferTexture(gbuffer, GL_COLOR_ATTACHMENT1, mDiffuseBufferTex, 0);
		glNamedFramebufferTexture(gbuffer, GL_COLOR_ATTACHMENT2, mSpecularBufferTex, 0);

		GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(gbuffer, 3, draw_bufs);

		GLenum status = glCheckNamedFramebufferStatus(gbuffer, GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != status) {
			assert(false);
			return false;
		}

		mDeferredGBuffers = CreateRenderTarget();
		mDeferredGBuffers->Set(gbuffer);

		mDiffuseBufferTexHandle = glGetTextureHandleARB(mDiffuseBufferTex);
		mNormalBufferTexHandle = glGetTextureHandleARB(mNormalBufferTex);
		mSpecularBufferTexHandle = glGetTextureHandleARB(mSpecularBufferTex);
		mDepthBufferTexHandle = glGetTextureHandleARB(mDepthBufferTex);

		glMakeTextureHandleResidentARB(mDiffuseBufferTexHandle);
		glMakeTextureHandleResidentARB(mNormalBufferTexHandle);
		glMakeTextureHandleResidentARB(mSpecularBufferTexHandle);
		glMakeTextureHandleResidentARB(mDepthBufferTexHandle);

		/****************deferred shading + ssao fb**********************/

		GLuint deferredShadingRT;
		glCreateFramebuffers(1, &deferredShadingRT);

		GenerateColorTex(mDeferredShadingTex);
		GenerateColorTex(mSsaoTex);
		GenerateDepthTex(mDeferredShadingDepthTex);

		glNamedFramebufferTexture(deferredShadingRT, GL_DEPTH_ATTACHMENT, mDeferredShadingDepthTex, 0);
		glNamedFramebufferTexture(deferredShadingRT, GL_COLOR_ATTACHMENT0, mDeferredShadingTex, 0);
		glNamedFramebufferTexture(deferredShadingRT, GL_COLOR_ATTACHMENT1, mSsaoTex, 0);

		GLenum shading_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(deferredShadingRT, 2, shading_bufs);

		status = glCheckNamedFramebufferStatus(deferredShadingRT, GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != status) {
			assert(false);
			return false;
		}

		mDeferredShadingRT = CreateRenderTarget();
		mDeferredShadingRT->Set(deferredShadingRT);

		mDeferredShadingTexHandle = glGetTextureHandleARB(mDeferredShadingTex);
		mSsaoTexHandle = glGetTextureHandleARB(mSsaoTex);
		mDeferredShadingDepthTexHandle = glGetTextureHandleARB(mDeferredShadingDepthTex);

		glMakeTextureHandleResidentARB(mDeferredShadingTexHandle);
		glMakeTextureHandleResidentARB(mSsaoTexHandle);
		glMakeTextureHandleResidentARB(mDeferredShadingDepthTexHandle);

		/******************************************************************/

		for (uint32_t i = 0; i < 4; ++i)
		{
			GenerateColorTex(mComputeOutput[i]);
			mComputeOutputHandle[i] = glGetTextureHandleARB(mComputeOutput[i]);
			glMakeTextureHandleResidentARB(mComputeOutputHandle[i]);
		}

		GLuint randVecTex = CreateGLTextureFromFile("Model/RandNorm.dds");
		glTextureParameteri(randVecTex, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTextureParameteri(randVecTex, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		mRandVecTexHandle = glGetTextureHandleARB(randVecTex);
		glMakeTextureHandleResidentARB(mRandVecTexHandle);

		DirectionalLight* dLight = new DirectionalLight();
		dLight->intensity = { 1.5f, 1.5f, 1.5f, 1 };
		dLight->direction = Vec3(-0.5, -2, 1).Normalized();
		dLight->castShadow = 1;

		LightManager::Instance()->m_dirLights.push_back(dLight);
		LightManager::Instance()->m_shadowMap = new ShadowMap(2048, 2048);

		mSkyBox = std::unique_ptr<Skybox>(new Skybox(std::wstring(L"Model/sunsetcube1024.dds")));

		InitScreenQuad();
		InitDebugCoord();

		mDrawTopologyMap[PrimitiveTopology::POINTLIST] = GL_POINTS;
		mDrawTopologyMap[PrimitiveTopology::LINELIST] = GL_LINES;
		mDrawTopologyMap[PrimitiveTopology::TRIANGLELIST] = GL_TRIANGLES;
		mDrawTopologyMap[PrimitiveTopology::CONTROL_POINT_3_PATCHLIST] = GL_PATCHES;
		mDrawTopologyMap[PrimitiveTopology::CONTROL_POINT_4_PATCHLIST] = GL_PATCHES;


		EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::RENDER, [this]() {Draw(); });

		mReadIndex = 0;
		mWriteIndex = 1;

		return true;
	}

	void AcquireContext() final
	{
		if (mContextThreadId == std::this_thread::get_id())
		{
			return;
		}
		else
		{
			const std::thread::id zeroID;
			if (mContextThreadId == zeroID)
			{
				mContextThreadId = std::this_thread::get_id();
				wglMakeCurrent(mDeviceContext, mRenderingContext);
			}
			else
			{
				assert(0);
			}
		}

	}

	void ReleaseContext() final
	{
		assert(mContextThreadId == std::this_thread::get_id());
		const std::thread::id zeroID;
		mContextThreadId = zeroID;
		wglMakeCurrent(nullptr, nullptr);
	}

	void Draw() final {
		EnterFrame();

		DrawShadowMap();
		DrawGBuffer();
		DrawDeferredShading();
		mSkyBox->Draw();
		DrawBlurSSAOAndCombine();
		//// TODO put every graphics call into cmdlist
		//
		////DrawGodRay();
		DrawDebug();
		
		// TEST
		ImGui::NewFrame();
		//ImGui::ShowTestWindow();
		GameObjectManager::Instance()->DrawMenu();
		SceneFileManager::Instance()->DrawMenu();
		
		
		if (ImGui::Begin("Render Settings"))
		{
			ImGui::Checkbox("SSAO", &mEnableSSAO);
			ImGui::End();
		}
		
		ImGui::Render();

		gFinishedDrawListSync.wait();
		mWriteIndex = std::atomic_exchange(&mReadIndex, mWriteIndex);
		gPrepareDrawListSync.notify();
	}

	float GetAspectRatio() final {
		return static_cast<float>(mClientWidth) / static_cast<float>(mClientHeight);
	}

	int GetScreenWidth() final {
		return mClientWidth;
	}

	int GetScreenHeight() final {
		return mClientHeight;
	}

	Skybox* GetSkyBox() final {
		return mSkyBox.get();
	}

	void* GetDevice() final {
		return nullptr;
	}

	void* GetDeviceContext() final {
		return nullptr;
	}

	void* GetDepthRS() final {
		return nullptr;
	}

	void UpdateTitle(const char* str) final {
		SetWindowTextA(mHMainWnd, str);
	}

	RenderTarget &GetGbuffer() final {
		return mDeferredGBuffers;
	}

	void EndFrame() final
	{
		ExecuteCmdList();

		SwapBuffers(mDeviceContext);

		gFinishedDrawListSync.notify();
	}

	void DrawShadowMap() final
	{
		LightManager::Instance()->m_shadowMap->UpdateShadowMatrix();

		uint32_t width, height;
		LightManager::Instance()->m_shadowMap->GetDimension(width, height);

		DrawCmd shadowcmd;

		shadowcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH;
		shadowcmd.framebuffer = LightManager::Instance()->m_shadowMap->GetRenderTarget();
		shadowcmd.viewport = { 0, 0, (float)width, (float)height, 0, 1 };

		AddDeferredDrawCmd(std::move(shadowcmd));

		for (auto &gatherShadowDrawCall : mShadowDrawHandler)
		{
			gatherShadowDrawCall();
		}
	}

	void DrawGBuffer() final
	{
		DrawCmd drawcmd;

		drawcmd.flags = CmdFlag::BIND_FB | CmdFlag::SET_VP | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH;
		drawcmd.framebuffer = mDeferredGBuffers;
		drawcmd.viewport = { 0.f, 0.f, (float)mClientWidth, (float)mClientHeight, 0.f, 1.f };

		AddDeferredDrawCmd(std::move(drawcmd));

		for (auto &gatherDrawCall : mDrawHandler)
		{
			gatherDrawCall();
		}
	}

	void DrawDeferredShading() final {
		DrawCmd cmd;
		DeferredShadingPassEffect* effect = EffectsManager::Instance()->m_deferredShadingPassEffect.get();

		ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
		DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER* perFrameData = (DeferredShadingPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();

		ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredShadingPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
		DeferredShadingPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredShadingPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer1->GetBuffer();


		textureData->NormalGMap = mNormalBufferTexHandle;
		textureData->DiffuseGMap = mDiffuseBufferTexHandle;//LightManager::Instance()->m_shadowMap->GetDepthTex();//
		textureData->SpecularGMap = mSpecularBufferTexHandle;
		textureData->DepthGMap = mDepthBufferTexHandle;
		textureData->RandVectMap = mRandVecTexHandle;

		DirectionalLight viewDirLight;
		memcpy(&viewDirLight, LightManager::Instance()->m_dirLights[0], sizeof(DirectionalLight));

		Mat4 ViewInvTranspose = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix().Inverse().Transpose();

		viewDirLight.direction = (ViewInvTranspose * Vec4(viewDirLight.direction.data[0], viewDirLight.direction.data[1], viewDirLight.direction.data[2], 0)).xyz();
		perFrameData->gDirLight = viewDirLight;

		Vec4 camPos = CameraManager::Instance()->GetActiveCamera()->GetPos();
		camPos = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * camPos;
		perFrameData->gEyePosV = camPos;
		perFrameData->gProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetProjMatrix());
		perFrameData->gProjInv = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetProjMatrix().Inverse());

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = (void*)mScreenQuadVAO;
		// cmd.vertexBuffer.push_back(0); // don't bind if 0
		// cmd.vertexOffset.push_back(0);
		// cmd.vertexStride.push_back(0);
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = mDeferredShadingRT;
		cmd.offset = (void*)(0);
		cmd.effect = effect;
		cmd.elementCount = 6;
		cmd.cbuffers.push_back(std::move(cbuffer0));
		cmd.cbuffers.push_back(std::move(cbuffer1));

		AddDeferredDrawCmd(std::move(cmd));
	}

	void DrawBlurSSAOAndCombine() final {
		if (mEnableSSAO)
		{
			doCSBlur(mSsaoTex, 0);
		}

		// ------ Screen Quad -------//
		// BADLY NAMED, this is actually just a shader to multiply SSAO effect
		BlurEffect* blurEffect = EffectsManager::Instance()->m_blurEffect.get();

		DrawCmd cmd;

		// TODO
		//m_d3d11DeviceContext->PSSetSamplers(0, 1, samplerState);

		cmd.flags = CmdFlag::DRAW | CmdFlag::CLEAR_COLOR | CmdFlag::CLEAR_DEPTH | CmdFlag::BIND_FB;
		cmd.drawType = DrawType::ARRAY;
		cmd.inputLayout = (void*)mScreenQuadVAO;
		// cmd.vertexBuffer.push_back(0); // don't bind if 0
		// cmd.vertexOffset.push_back(0);
		// cmd.vertexStride.push_back(0);
		cmd.type = PrimitiveTopology::TRIANGLELIST;
		cmd.framebuffer = mDefaultRT;
		cmd.offset = (void*)(0);
		cmd.effect = blurEffect;
		cmd.elementCount = 6;

		ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(BlurEffect::SETTING_CONSTANT_BUFFER), blurEffect->m_settingCB);
		BlurEffect::SETTING_CONSTANT_BUFFER* settingData = (BlurEffect::SETTING_CONSTANT_BUFFER*)cbuffer0->GetBuffer();

		ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(BlurEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), blurEffect->m_textureCB);
		BlurEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (BlurEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

		textureData->ScreenMap = mDeferredShadingTexHandle;
		if (mEnableSSAO)
		{
			textureData->SSAOMap = mComputeOutputHandle[1];
			textureData->DepthMap = mDepthBufferTexHandle;
		}
		//settingData->BloomMap = NOT_USED;
		settingData->xEnableSSAO.x() = mEnableSSAO;

		cmd.cbuffers.push_back(std::move(cbuffer0));
		cmd.cbuffers.push_back(std::move(cbuffer1));

		AddDeferredDrawCmd(cmd);
	}

	void DrawDebug() final {
		DebugLineEffect* debugLineFX = EffectsManager::Instance()->m_debugLineEffect.get();

		DrawCmd cmd;

		cmd.flags = /*CmdFlag::BIND_FB |*/ CmdFlag::SET_DS | CmdFlag::DRAW;

		cmd.depthState.depthWriteEnable = false;
		cmd.effect = debugLineFX;
		cmd.inputLayout = (void*)mDebugCoordVAO;

		ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DebugLineEffect::PEROBJ_CONSTANT_BUFFER), debugLineFX->m_perObjectCB);
		DebugLineEffect::PEROBJ_CONSTANT_BUFFER* perObjectData = (DebugLineEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer0->GetBuffer();

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

	~GLRenderer()
	{
		glMakeTextureHandleNonResidentARB(mDiffuseBufferTexHandle);
		glMakeTextureHandleNonResidentARB(mNormalBufferTexHandle);
		glMakeTextureHandleNonResidentARB(mSpecularBufferTexHandle);
		glMakeTextureHandleNonResidentARB(mDepthBufferTexHandle);
	}

	void AddDeferredDrawCmd(DrawCmd &cmd) final
	{
		//m_deferredDrawList.push_back(std::move(cmd));
		mDrawList[mWriteIndex].push_back(std::move(cmd));
	}

	void AddDraw(DrawEventHandler handler) final
	{
		mDrawHandler.push_back(handler);
	}

	void AddShadowDraw(DrawEventHandler handler) final
	{
		mShadowDrawHandler.push_back(handler);
	}

	ConstantBuffer CreateConstantBuffer(uint32_t index, uint32_t size, GPUBuffer buffer) final
	{
		return std::make_unique<GLConstantBuffer>(index, size, buffer);
	}

	GPUBuffer CreateGPUBuffer(size_t size, BufferUsage usage, void* data /*= nullptr*/, BufferType type /*= BufferType::GENERAL*/) final
	{
		return std::make_shared<GLBuffer>(size, usage, data, type);
	}

	RenderTarget CreateRenderTarget() final
	{
		return std::make_shared<GLRenderTarget>();
	}

	UAVBinding CreateUAVBinding() final
	{
		return std::make_unique<GLUAVBinding>();
	}

	Texture CreateTexture(uint32_t width, uint32_t height, void* srv) final
	{
		return std::make_shared<GLTexture>(width, height, srv);
	}

private:

	bool initializeExtensions()
	{
		HDC deviceContext;
		PIXELFORMATDESCRIPTOR pixelFormat;
		int error;
		HGLRC renderContext;
		//bool result;

		deviceContext = GetDC(mHMainWnd);
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

		ReleaseDC(mHMainWnd, deviceContext);
		deviceContext = 0;

		return true;
	}

	bool initializeOpenGL()
	{
		int pixelFormat[1];
		unsigned int formatCount;
		int result;
		PIXELFORMATDESCRIPTOR pixelFormatDescriptor;

		mDeviceContext = GetDC(mHMainWnd);
		if (!mDeviceContext)
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

		result = wglChoosePixelFormatARB(mDeviceContext, attributeListInt, NULL, 1, pixelFormat, &formatCount);
		if (result != 1)
		{
			return false;
		}

		result = SetPixelFormat(mDeviceContext, pixelFormat[0], &pixelFormatDescriptor);
		if (result != 1)
		{
			return false;
		}

		int attributeList[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 5,
#if !BUILD_RELEASE
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
			0
		};

		mRenderingContext = wglCreateContextAttribsARB(mDeviceContext, 0, attributeList);
		if (mRenderingContext == NULL)
		{
			assert(false);
			return false;
		}

		mRenderingContext = wglCreateContextAttribsARB(mDeviceContext, 0, attributeList);
		if (mRenderingContext == NULL)
		{
			assert(false);
			return false;
		}

		AcquireContext();

		return true;
	}

	void ExecuteCmdList()
	{
		for (auto &cmd : mDrawList[mReadIndex])
		{
			if (cmd.flags & CmdFlag::BIND_FB)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)cmd.framebuffer->Get());
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
				if (cmd.rasterizerState.cullFaceEnabled > 0)
				{
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
					glEnable(GL_CULL_FACE);
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
					cbuffer->Bind();
				}

				if (cmd.flags & CmdFlag::DRAW)
				{
					if (cmd.inputLayout)
					{
						glBindVertexArray((GLuint)cmd.inputLayout);
					}

					for (auto i = 0; i < cmd.vertexBuffer.size(); i++)
					{
						glBindVertexBuffer(i, (GLuint)cmd.vertexBuffer[i], cmd.vertexOffset[i], cmd.vertexStride[i]);
					}
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
						{
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)cmd.indexBuffer);
						}

						if (cmd.instanceCount)
						{
							glDrawElementsInstanced(
								mDrawTopologyMap[cmd.type],
								cmd.elementCount,
								GL_UNSIGNED_INT,
								cmd.offset,
								cmd.instanceCount
								);
						}
						else
						{
							glDrawElements(
								mDrawTopologyMap[cmd.type],
								cmd.elementCount,
								GL_UNSIGNED_INT,
								cmd.offset
							);
						}
					}
					else if (cmd.drawType == DrawType::ARRAY)
					{
						if (cmd.instanceCount)
						{
							assert(0);
						}
						else
						{
							glDrawArrays(mDrawTopologyMap[cmd.type], (GLint)cmd.offset, cmd.elementCount);
						}
					}
				}
				else if (cmd.flags & CmdFlag::COMPUTE)
				{
					cmd.uavs->Bind();
					glDispatchCompute​(cmd.threadGroupX, cmd.threadGroupY, cmd.threadGroupZ);
				}
			}

			if (cmd.imGuiIbo)
			{
				glNamedBufferData(cmd.imGuiIbo, cmd.imGuiIdxBuffer.size() * sizeof(ImDrawIdx), &cmd.imGuiIdxBuffer.front(), GL_STREAM_DRAW);
			}
			if (cmd.imGuiVbo)
			{
				glNamedBufferData(cmd.imGuiVbo, cmd.imGuiVtxBuffer.size() * sizeof(ImDrawVert), &cmd.imGuiVtxBuffer.front(), GL_STREAM_DRAW);
			}
		}

		mDrawList[mReadIndex].clear();
	}

	void EnterFrame()
	{
		// reset state
		DrawCmd cmd;
		cmd.flags = CmdFlag::SET_VP | CmdFlag::SET_BS | CmdFlag::SET_CS | CmdFlag::SET_SS | CmdFlag::SET_DS | CmdFlag::CLEAR_COLOR /*| CmdFlag::CLEAR_DEPTH*/;
		cmd.viewport.TopLeftX = 0;
		cmd.viewport.TopLeftY = 0;
		cmd.viewport.Width = mClientWidth;
		cmd.viewport.Height = mClientHeight;

		AddDeferredDrawCmd(cmd);
	}

	void doCSBlur(GLuint blurImgSRV, int uavSlotIdx)
	{
		// vblur
		DrawCmd cmdV;

		VBlurEffect* vBlurEffect = EffectsManager::Instance()->m_vblurEffect.get();
		UINT numGroupsX = (UINT)ceilf(mClientWidth / 256.0f);

		cmdV.effect = vBlurEffect;
		cmdV.flags = CmdFlag::COMPUTE;
		cmdV.threadGroupX = numGroupsX;
		cmdV.threadGroupY = mClientHeight;
		cmdV.threadGroupZ = 1;
		cmdV.uavs = CreateUAVBinding();
		cmdV.uavs->AddUAV(reinterpret_cast<void*>(blurImgSRV), 0);
		cmdV.uavs->AddUAV(reinterpret_cast<void*>(mComputeOutput[uavSlotIdx]), 1);

		AddDeferredDrawCmd(std::move(cmdV));

		// hblur
		DrawCmd cmdH;

		HBlurEffect* hBlurEffect = EffectsManager::Instance()->m_hblurEffect.get();
		UINT numGroupsY = (UINT)ceilf(mClientHeight / 256.0f);

		cmdH.effect = hBlurEffect;
		cmdH.flags = CmdFlag::COMPUTE;
		cmdH.threadGroupX = mClientWidth;
		cmdH.threadGroupY = numGroupsY;
		cmdH.threadGroupZ = 1;
		cmdH.uavs = CreateUAVBinding();
		cmdH.uavs->AddUAV(reinterpret_cast<void*>(mComputeOutput[uavSlotIdx]), 0);
		cmdH.uavs->AddUAV(reinterpret_cast<void*>(mComputeOutput[uavSlotIdx + 1]), 1);

		AddDeferredDrawCmd(std::move(cmdH));
	}

	void GenerateColorTex(GLuint &bufferTex) 
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &bufferTex);
		glTextureStorage2D(bufferTex, 1, GL_RGBA16F, mClientWidth, mClientHeight);

		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(bufferTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(bufferTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	void GenerateDepthTex(GLuint &bufferTex) 
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &bufferTex);
		glTextureStorage2D(bufferTex, 1, GL_DEPTH_COMPONENT32, mClientWidth, mClientHeight);

		glTextureParameteri(bufferTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(bufferTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(bufferTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void InitScreenQuad()
	{
		// init screen quad vbo
		std::vector<Vec4> quadVertexBuffer = {
			Vec4(-1.0, -1.0, -1.0, 1.0),
			Vec4(-1.0, 1.0, -1.0, 1.0),
			Vec4(1.0, 1.0, -1.0, 1.0),
			Vec4(1.0, 1.0, -1.0, 1.0),
			Vec4(1.0, -1.0, -1.0, 1.0),
			Vec4(-1.0, -1.0, -1.0, 1.0),
		};

		std::vector<Vec2> quadUvVertexBuffer = {
			Vec2(0, 0),
			Vec2(0, 1),
			Vec2(1, 1),
			Vec2(1, 1),
			Vec2(1, 0),
			Vec2(0, 0),
		};

		GLuint screenQuadVertexVBO;
		GLuint screenQuadUVVBO;
		glCreateBuffers(1, &screenQuadVertexVBO);
		glNamedBufferStorage(screenQuadVertexVBO, quadVertexBuffer.size() * sizeof(Vec4), &quadVertexBuffer[0], 0);

		glCreateBuffers(1, &screenQuadUVVBO);
		glNamedBufferStorage(screenQuadUVVBO, quadUvVertexBuffer.size() * sizeof(Vec2), &quadUvVertexBuffer[0], 0);

		glCreateVertexArrays(1, &mScreenQuadVAO);

		glEnableVertexArrayAttrib(mScreenQuadVAO, 0);
		glEnableVertexArrayAttrib(mScreenQuadVAO, 1);

		glVertexArrayVertexBuffer(mScreenQuadVAO, 0, screenQuadVertexVBO, 0, sizeof(Vec4));
		glVertexArrayVertexBuffer(mScreenQuadVAO, 1, screenQuadUVVBO, 0, sizeof(Vec2));

		glVertexArrayAttribFormat(mScreenQuadVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(mScreenQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, 0);

		glVertexArrayAttribBinding(mScreenQuadVAO, 0, 0);
		glVertexArrayAttribBinding(mScreenQuadVAO, 1, 1);
	}

	void InitDebugCoord()
	{
		// init grid and coord debug draw
		std::vector<Vec3Packed> coordVertexBuffer = {
			Vec3Packed({ 0, 0, 0 }),
			Vec3Packed({ 5, 0, 0 }),
			Vec3Packed({ 0, 0, 0 }),
			Vec3Packed({ 0, 5, 0 }),
			Vec3Packed({ 0, 0, 0 }),
			Vec3Packed({ 0, 0, 5 }),
		};

		std::vector<UINT> coordIndexBuffer = { 0, 1, 2, 3, 4, 5 };

		int initIdx = 6;
		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.emplace_back(Vec3(-5.f, 0.f, -5.f + i));
			coordVertexBuffer.emplace_back(Vec3(5.f, 0.f, -5.f + i));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		for (int i = 0; i <= 10; i++) {
			coordVertexBuffer.emplace_back(Vec3(-5.f + i, 0.f, -5.f));
			coordVertexBuffer.emplace_back(Vec3(-5.f + i, 0.f, 5.f));
			coordIndexBuffer.push_back(initIdx++);
			coordIndexBuffer.push_back(initIdx++);
		}

		GLuint debugDrawVertexVBO;
		glCreateBuffers(1, &debugDrawVertexVBO);
		glNamedBufferStorage(debugDrawVertexVBO, coordVertexBuffer.size() * sizeof(Vec3Packed), &coordVertexBuffer[0], 0);

		glCreateVertexArrays(1, &mDebugCoordVAO);

		glEnableVertexArrayAttrib(mDebugCoordVAO, 0);
		glVertexArrayAttribFormat(mDebugCoordVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayVertexBuffer(mDebugCoordVAO, 0, debugDrawVertexVBO, 0, sizeof(Vec3Packed));
		glVertexArrayAttribBinding(mDebugCoordVAO, 0, 0);
	}

	int32_t mClientWidth;
	int32_t mClientHeight;
	bool mEnable4xMsaa;
	std::unique_ptr<Skybox> mSkyBox;

	HINSTANCE	mHInst;
	HWND		mHMainWnd;
	HDC			mDeviceContext;
	HGLRC		mRenderingContext;

	RenderTarget mDeferredGBuffers;

	GLuint mDiffuseBufferTex;
	GLuint mNormalBufferTex;
	GLuint mSpecularBufferTex;
	GLuint mDepthBufferTex;

	RenderTarget mDeferredShadingRT;;
	RenderTarget mDefaultRT;;

	GLuint mSsaoTex;
	GLuint mDeferredShadingTex;
	GLuint mDeferredShadingDepthTex;

	GLuint mComputeOutput[4];
	uint64_t mComputeOutputHandle[4];

	uint64_t mRandVecTexHandle;

	uint64_t mDiffuseBufferTexHandle;
	uint64_t mNormalBufferTexHandle;
	uint64_t mSpecularBufferTexHandle;
	uint64_t mDepthBufferTexHandle;

	uint64_t mSsaoTexHandle;
	uint64_t mDeferredShadingTexHandle;
	uint64_t mDeferredShadingDepthTexHandle;

	GLuint mDebugCoordVAO;
	GLuint mScreenQuadVAO;

	std::vector<DrawCmd> mDrawList[2];
	std::atomic<uint8_t> mReadIndex;
	std::atomic<uint8_t> mWriteIndex;

	Semaphore &gPrepareDrawListSync;
	Semaphore &gFinishedDrawListSync;

	std::thread::id mContextThreadId;

	uint64_t mCurrentVao;
	Effect* mCurrentEffect;
	uint64_t mCurrentFbo;

	std::vector<DrawEventHandler> mDrawHandler;
	std::vector<DrawEventHandler> mShadowDrawHandler;

	std::unordered_map<PrimitiveTopology, GLenum> mDrawTopologyMap;
	bool mEnableSSAO;
};

Renderer* CreateGLRenderer(HINSTANCE hInstance, HWND hMainWnd, Semaphore &prepareDrawListSync, Semaphore &finishedDrawListSync)
{
	auto renderer = new GLRenderer(hInstance, hMainWnd, prepareDrawListSync, finishedDrawListSync);
	return static_cast<Renderer*>(renderer);
}

}