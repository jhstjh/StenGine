#include "LightManager.h"
#include "ShadowMap.h"

LightManager* LightManager::_instance = nullptr;

LightManager::~LightManager() {
	for (int i = 0; i < m_dirLights.size(); i++) {
		delete m_dirLights[i];
	}
}

