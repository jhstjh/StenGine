#pragma once
#include "System/API/PlatformAPIDefs.h"

#if GRAPHICS_D3D11
#include "Graphics/D3DIncludes.h"
#endif

namespace StenGine
{

#if GRAPHICS_OPENGL
struct Viewport
{
	float TopLeftX;
	float TopLeftY;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;	
};
#endif

#if GRAPHICS_D3D11
using Viewport = D3D11_VIEWPORT;
#endif

}