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

	
	void DrawGBuffer();
	void DrawDeferredShading();
	void DrawBlurSSAOAndCombine();
	void DrawDebug();
	void DrawGodRay();

private:
	static GLRenderer* _instance;



	void GenerateColorTex(GLuint &bufferTex);
	void GenerateDepthTex(GLuint &bufferTex);
};

#endif