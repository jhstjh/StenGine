#ifndef __MATERIAL__
#define __MATERIAL__
#ifdef PLATFORM_WIN32
#include "D3DIncludes.h"
#endif

class Material {
public:
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	XMFLOAT4 roughness_metalic_c_doublesided; // cook-torrence-param
};

#endif