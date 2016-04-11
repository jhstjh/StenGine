#pragma once

#ifdef GRAPHICS_OPENGL
#undef GRAPHICS_OPENGL
#define GRAPHICS_OPENGL 1
#else
#define GRAPHICS_OPENGL 0
#endif

#ifdef GRAPHICS_D3D11
#undef GRAPHICS_D3D11
#define GRAPHICS_D3D11 1
#else
#define GRAPHICS_D3D11 0
#endif

#ifdef WIN32
#define PLATFORM_WIN32 1
#elif 
#define PLATFORM_WIN32 0
#endif

#ifdef _ANDROID_
#define PLATFORM_ANDROID 1
#else
#define PLATFORM_ANDROID 0
#endif