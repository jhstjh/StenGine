#ifndef __MATERIAL__
#define __MATERIAL__

#include "Graphics/Abstraction/Texture.h"

namespace StenGine
{

class Material {
public:
	struct MaterialAttrib
	{
		Vec4Packed ambient{ { 0.2f, 0.2f, 0.2f, 1.f } };
		Vec4Packed diffuse { { 1.0f, 0.8f, 0.7f, 1.f } };
		Vec4Packed specular { { 0.6f, 0.6f, 0.6f, 10.0f } };

		Vec4Packed roughness_metalic_c_doublesided { { 0.1f, 1.0f, 0.6f, 0.0f } };
	} m_attributes;

	Texture m_diffuseMapTex = nullptr;
	Texture m_normalMapTex = nullptr;
	Texture m_bumpMapTex = nullptr;
};

}
#endif