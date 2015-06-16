#include "LightManager.h"

#ifdef PLATFORM_WIN32
#include "ShadowMap.h"
#endif

LightManager* LightManager::_instance = nullptr;

LightManager::~LightManager() {
	for (int i = 0; i < m_dirLights.size(); i++) {
		delete m_dirLights[i];
	}
}

