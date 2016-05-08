#ifndef __MATERIAL__
#define __MATERIAL__

#include "System/API/PlatformAPIDefs.h"
#include "Graphics/Abstraction/Texture.h"

#if PLATFORM_WIN32
#include "Graphics/D3DIncludes.h"
#endif

namespace StenGine
{

class Material {
public:
	struct MaterialAttrib
	{
		XMFLOAT4 ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);;
		XMFLOAT4 diffuse = XMFLOAT4(1.0f, 0.8f, 0.7f, 1.f);
		XMFLOAT4 specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 10.0f);

		XMFLOAT4 roughness_metalic_c_doublesided = { 0.1f, 1.0f, 0.6f, 0.0f };
	} m_attributes;

	Texture* m_diffuseMapTex = nullptr;
	Texture* m_normalMapTex = nullptr;
	Texture* m_bumpMapTex = nullptr;

};

}
#endif