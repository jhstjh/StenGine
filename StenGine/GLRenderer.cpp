#include "GLRenderer.h"
#include "D3DIncludes.h"
#include "EffectsManager.h"
#include "MeshRenderer.h"
#include "LightManager.h"
#include "MathHelper.h"
#include "CameraManager.h"
#include "Color.h"


GLRenderer* GLRenderer::_instance = nullptr;

GLRenderer::GLRenderer(HINSTANCE hInstance, HWND hMainWnd) :
m_hInst(hInstance),
m_hMainWnd(hMainWnd)
{
	m_clientWidth = 1280;
	m_clientHeight = 720;
	_instance = this;
}

bool GLRenderer::Init() {
	m_deviceContext = GetWindowDC(m_hMainWnd);
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nPixelFormat = ChoosePixelFormat(m_deviceContext, &pfd);

	assert(nPixelFormat != 0, "Error choosing pixel format");

	BOOL bResult = SetPixelFormat(m_deviceContext, nPixelFormat, &pfd);

	assert(bResult != 0, "Error setting pixel format");

	HGLRC tempContext = wglCreateContext(m_deviceContext);
	wglMakeCurrent(m_deviceContext, tempContext);

	GLenum glewerr = glewInit();
	if (GLEW_OK != glewerr)
	{
		OutputDebugStringA("GLEW is not initialized!\n");
		return false;
	}

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,

		//WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,                

		// 			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		// 			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		// 			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		// 			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,

		//WGL_COLOR_BITS_ARB, 32,
		//WGL_DEPTH_BITS_ARB, 32,
		//WGL_STENCIL_BITS_ARB, 8,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};

	m_renderingContext = wglCreateContextAttribsARB(m_deviceContext, 0, attribs);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tempContext);
	wglMakeCurrent(m_deviceContext, m_renderingContext);

	//Checking GL version
	const GLubyte *GLVersionString = glGetString(GL_VERSION);

	//Or better yet, use the GL3 way to get the version number
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

	assert(m_renderingContext, "Error creating gl context");


	glClearColor(0.2, 0.2, 0.2, 0);
	glClearDepth(1.0f);


	m_renderingContext = wglGetCurrentContext();
	m_deviceContext = wglGetCurrentDC();
	bool noError = wglMakeCurrent(m_deviceContext, m_renderingContext);
	assert(noError);
	
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CW); // GL_CCW for counter clock-wise
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

	return true;
}

void GLRenderer::Draw() {

	LightManager::Instance()->m_shadowMap->RenderShadowMap();

	/**************deferred shading 1st pass******************/
	glViewport(0, 0, m_clientWidth, m_clientHeight);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_deferredGBuffers);
	EffectsManager::Instance()->m_deferredGeometryPassEffect->SetShader();
	DeferredGeometryPassEffect* effect = EffectsManager::Instance()->m_deferredGeometryPassEffect;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: should separate perobj and perframe's updateconstantbuffer

	effect->m_perFrameUniformBuffer.EyePosW = (CameraManager::Instance()->GetActiveCamera()->GetPos());
	effect->m_perFrameUniformBuffer.DirLight = *LightManager::Instance()->m_dirLights[0];
	effect->ShadowMapTex = LightManager::Instance()->m_shadowMap->GetDepthTex();
	effect->CubeMapTex = m_SkyBox->m_cubeMapTex;

	for (int iMesh = 0; iMesh < EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes.size(); iMesh++) {
		EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes[iMesh]->Draw();
	}

	EffectsManager::Instance()->m_deferredGeometryPassEffect->UnSetShader();

	/**************deferred shading 2nd pass*****************/
	
	DeferredShadingPassEffect* deferredShadingFX = EffectsManager::Instance()->m_deferredShadingPassEffect;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_SkyBox->Draw();


	deferredShadingFX->SetShader();
	glBindVertexArray(NULL);

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
	deferredShadingFX->UnSetShader();
	deferredShadingFX->UnBindShaderResource();
	

	SwapBuffers(m_deviceContext);
}

GLRenderer::~GLRenderer() {}

void GLRenderer::GenerateColorTex(GLuint &bufferTex) {
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

void GLRenderer::GenerateDepthTex(GLuint &bufferTex) {
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