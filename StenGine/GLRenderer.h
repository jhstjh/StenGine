#ifndef __GLRENDERER__
#define __GLRENDERER__

#include "stdafx.h"
#include "MeshRenderer.h"
#include "Skybox.h"
#include "RendererBase.h"

class GLRenderer: public Renderer {
public: 
	GLRenderer(HINSTANCE hInstance, HWND hMainWnd);
	virtual ~GLRenderer();
	static GLRenderer* Instance() { return _instance; }
	virtual bool Init();
	virtual void Draw();
private:
	static GLRenderer* _instance;

	HINSTANCE	m_hInst;
	HWND		m_hMainWnd;
	HDC			m_deviceContext;
	HGLRC		m_renderingContext;
};



#endif