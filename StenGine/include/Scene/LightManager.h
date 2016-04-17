#ifndef __LIGHT_MANAGER__
#define __LIGHT_MANAGER__

#include "System/API/PlatformAPIDefs.h"
#include "System/SingletonClass.h"

#if PLATFORM_WIN32
#include "Graphics/D3DIncludes.h"
#elif  PLATFORM_ANDROID
#include "AndroidType.h"
#endif

namespace StenGine
{

struct DirectionalLight {
	//DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 intensity;
	XMFLOAT3 direction;
	float castShadow;
};


class LightManager : public SingletonClass<LightManager> 
{
public:
	~LightManager();
	std::vector<DirectionalLight*> m_dirLights;

#if PLATFORM_WIN32
	class ShadowMap* m_shadowMap;
#endif
};

}

#endif // !__LIGHT_MANAGER__
