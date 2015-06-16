#ifndef __LIGHT_MANAGER__
#define __LIGHT_MANAGER__
#ifdef PLATFORM_WIN32
#include "D3DIncludes.h"
#elif defined PLATFORM_ANDROID
#include "AndroidType.h"
#endif

struct DirectionalLight {
	//DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 intensity;
	XMFLOAT3 direction;
	float castShadow;
};


class LightManager {
private:
	static LightManager* _instance;
public:
	~LightManager();
	std::vector<DirectionalLight*> m_dirLights;
	static LightManager* Instance() {
		if (!_instance)
			_instance = new LightManager();
		return _instance;
	}
#ifdef PLATFORM_WIN32
	class ShadowMap* m_shadowMap;
#endif
};



#endif // !__LIGHT_MANAGER__
