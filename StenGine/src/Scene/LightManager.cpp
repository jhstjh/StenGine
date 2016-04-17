#include "Scene/LightManager.h"

#if PLATFORM_WIN32
#include "Graphics/Effect/ShadowMap.h"
#endif

namespace StenGine
{

DEFINE_SINGLETON_CLASS(LightManager)

LightManager::~LightManager() {
	for (uint32_t i = 0; i < m_dirLights.size(); i++) {
		delete m_dirLights[i];
	}
}

}