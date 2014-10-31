#ifndef __MATERIAL__
#define __MATERIAL__
#include "D3DIncludes.h"

class Material {
public:
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
};

#endif