#include "stdafx.h"

#include "Resource/ResourceManager.h"

namespace StenGine
{

DEFINE_SINGLETON_CLASS(ResourceManager)

ResourceManager::~ResourceManager() {
	for (auto it = m_meshResourceMap.begin(); it != m_meshResourceMap.end(); ++it) {
		SafeDelete(it->second);
	}
	m_meshResourceMap.clear();

	m_textureResourceMap.clear();

	for (auto it = m_animationResourceMap.begin(); it != m_animationResourceMap.end(); ++it) {
		SafeDelete(it->second);
	}
	m_animationResourceMap.clear();
}

}