#pragma once

#if GRAPHICS_OPENGL
#include "Graphics/OpenGL/GLConstantBuffer.h"
#elif  GRAPHICS_D3D11
#include "Graphics/D3D11/D3D11ConstantBuffer.h"
#else
#error unsupported api
#endif