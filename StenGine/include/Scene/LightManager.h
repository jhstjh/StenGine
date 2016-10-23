#ifndef __LIGHT_MANAGER__
#define __LIGHT_MANAGER__

#include "System/API/PlatformAPIDefs.h"
#include "System/SingletonClass.h"

#include "Graphics/D3DIncludes.h"

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

	class ShadowMap* m_shadowMap;
};

}

#endif // !__LIGHT_MANAGER__
