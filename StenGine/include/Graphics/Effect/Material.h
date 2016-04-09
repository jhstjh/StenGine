#ifndef __MATERIAL__
#define __MATERIAL__

#include "System/API/PlatformAPIDefs.h"

#if PLATFORM_WIN32
#include "Graphics/D3DIncludes.h"
#endif

class Material {
public:
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	XMFLOAT4 roughness_metalic_c_doublesided; // cook-torrence-param
};

#endif