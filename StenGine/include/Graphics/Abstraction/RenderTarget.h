#pragma once

#include "System/API/PlatformAPIDefs.h"

#if GRAPHICS_D3D11
#include "Graphics/D3D11/D3D11RenderTarget.h"
#endif

#if GRAPHICS_OPENGL

#include "glew.h"
namespace StenGine
{
using RenderTarget = GLuint;
}
#endif

