#pragma once

#include "System/API/PlatformAPIDefs.h"

#if GRAPHICS_D3D11
#include "Graphics/D3D11/D3D11Buffer.h"
#elif GRAPHICS_OPENGL
#include "Graphics/OpenGL/GLBuffer.h"
#endif