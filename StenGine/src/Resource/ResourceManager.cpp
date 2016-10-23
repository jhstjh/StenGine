#include "Resource/ResourceManager.h"

namespace StenGine
{

DEFINE_SINGLETON_CLASS(ResourceManager)

ResourceManager::~ResourceManager() {
	for (auto it = m_meshResourceMap.begin(); it != m_meshResourceMap.end(); ++it) {
		SafeDelete(it->second);
	}
}

}