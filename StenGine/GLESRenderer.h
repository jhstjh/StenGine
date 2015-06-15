#pragma once

#include <jni.h>
#include <errno.h>

#include <vector>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include <cpu-features.h>

#define CLASS_NAME "android/app/NativeActivity"
#define APPLICATION_CLASS_NAME "com/sample/teapot/TeapotApplication"

#include "NDKHelper.h"
#include "RendererBase.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class GLESRenderer : public Renderer {
public:
	GLESRenderer();
	~GLESRenderer();
	static GLESRenderer* Instance() { return _instance; }

	virtual bool Init();
	virtual void Draw();
	
	void DrawDebug();

private:
	static GLESRenderer* _instance;

	GLuint m_debugCoordVAO;
};