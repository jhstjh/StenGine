#include "ResourceManager.h"

ResourceManager* ResourceManager::_instance = nullptr;

// template <typename T>
// void ResourceManager::GetResource(std::wstring path, T* resource) {
// 	if (is_same<T, Mesh>::value) {
// 		auto got = m_meshResourceMap.find(path);
// 		if (got == m_meshResourceMap.end()) {
// 			if (path == L"GenerateBox") {
// 				CreateBoxPrimitive(resource);
// 			}
// 			else if (path == L"GeneratePlane") {
// 				CreatePlanePrimitive(resource);
// 			}
// 		}
// 		else {
// 			//return *got;
// 		}
// 	}
// }
