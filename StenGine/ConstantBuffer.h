#pragma once

#ifdef GRAPHICS_OPENGL
#include "GLConstantBuffer.h"
#elif defined GRAPHICS_D3D11
#include "D3D11ConstantBuffer.h"
#else
#error unsupported api
#endif