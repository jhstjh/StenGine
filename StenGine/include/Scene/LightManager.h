#ifndef __LIGHT_MANAGER__
#define __LIGHT_MANAGER__

#include "Graphics/D3DIncludes.h"
#include "Math/MathDefs.h"
#include "System/API/PlatformAPIDefs.h"
#include "System/SingletonClass.h"

namespace StenGine
{

struct DirectionalLight {
	//DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	Vec4 intensity;
	Vec3Packed direction;
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
