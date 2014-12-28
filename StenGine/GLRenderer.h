#ifndef __GLRENDERER__
#define __GLRENDERER__

#include "stdafx.h"
#include "MeshRenderer.h"
#include "Skybox.h"
#include "RendererBase.h"
#include "GL/glew.h"
#include "GL/wglew.h"

class GLRenderer: public Renderer {
public: 
	GLRenderer(HINSTANCE hInstance, HWND hMainWnd);
	virtual ~GLRenderer();
	static GLRenderer* Instance() { return _instance; }
	virtual bool Init();
	virtual void Draw();
	Skybox* m_SkyBox;
	
private:
	static GLRenderer* _instance;

	HINSTANCE	m_hInst;
	HWND		m_hMainWnd;
	HDC			m_deviceContext;
	HGLRC		m_renderingContext;

	GLuint m_deferredGBuffers;

	GLuint m_diffuseBufferTex;
	GLuint m_normalBufferTex;
	GLuint m_specularBufferTex;
	GLuint m_depthBufferTex;

	void GenerateColorTex(GLuint &bufferTex);
	void GenerateDepthTex(GLuint &bufferTex);
};

#endif