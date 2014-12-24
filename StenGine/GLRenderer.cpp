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


	glClearColor(1, 0, 0, 0);
	glClearDepth(1.0f);


	m_renderingContext = wglGetCurrentContext();
	m_deviceContext = wglGetCurrentDC();
	bool noError = wglMakeCurrent(m_deviceContext, m_renderingContext);
	assert(noError);
	
	return true;
}

void GLRenderer::Draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SwapBuffers(m_deviceContext);
}

GLRenderer::~GLRenderer() {}