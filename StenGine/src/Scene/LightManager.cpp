#include "Scene/LightManager.h"

#if PLATFORM_WIN32
#include "Graphics/Effect/ShadowMap.h"
#endif

LightManager* LightManager::_instance = nullptr;

LightManager::~LightManager() {
	for (uint32_t i = 0; i < m_dirLights.size(); i++) {
		delete m_dirLights[i];
	}
}

