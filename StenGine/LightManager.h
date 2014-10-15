#ifndef __LIGHT_MANAGER__
#define __LIGHT_MANAGER__
#include "D3DIncludes.h"

struct DirectionalLight {
	//DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 intensity;
	XMFLOAT3 direction;
	float pad;
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
};



#endif // !__LIGHT_MANAGER__
