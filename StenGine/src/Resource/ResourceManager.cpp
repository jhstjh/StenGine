#include "Resource/ResourceManager.h"

namespace StenGine
{

DEFINE_SINGLETON_CLASS(ResourceManager)

ResourceManager::~ResourceManager() {
	for (auto it = m_meshResourceMap.begin(); it != m_meshResourceMap.end(); ++it) {
#if PLATFORM_WIN32
		SafeDelete(it->second);
#elif  PLATFORM_ANDROID
		if (it->second)
			delete it->second;
		it->second = 0;
#endif
	}

}

}