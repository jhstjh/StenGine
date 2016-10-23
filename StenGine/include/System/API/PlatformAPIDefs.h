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

#ifdef _DEBUG
#define BUILD_DEBUG 1
#else
#define BUILD_DEBUG 0
#endif

#ifdef NDEBUG
#define BUILD_RELEASE 1
#else
#define BUILD_RELEASE 0
#endif