#include "Resource/ResourceManager.h"

namespace StenGine
{

DEFINE_SINGLETON_CLASS(ResourceManager)

ResourceManager::~ResourceManager() {
	for (auto it = m_meshResourceMap.begin(); it != m_meshResourceMap.end(); ++it) {
		SafeDelete(it->second);
	}

	for (auto it = m_textureResourceMap.begin(); it != m_textureResourceMap.end(); ++it) {
		SafeDelete(it->second);
	}

	for (auto it = m_animationResourceMap.begin(); it != m_animationResourceMap.end(); ++it) {
		SafeDelete(it->second);
	}
}

}