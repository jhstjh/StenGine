#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__

#include <unordered_map>
#include "D3DIncludes.h"
#include "MeshRenderer.h"
#include <type_traits>
#include "FbxReaderSG.h"
/*class Mesh;*/

class ResourceManager {
public:
	static ResourceManager* Instance() {
		if (!_instance) {
			_instance = new ResourceManager();
		}
		return _instance;
	}

	template <typename T>
	T* GetResource(std::wstring path) {
		if (std::is_same<T, Mesh>::value) {
			auto got = m_meshResourceMap.find(path);
			if (got == m_meshResourceMap.end()) {
				if (path == L"GenerateBox") {
					Mesh* box = new Mesh(0);
					box->Prepare();
					m_meshResourceMap[L"GenerateBox"] = box;
					return box;
				}
				else if (path == L"GeneratePlane") {
					Mesh* plane = new Mesh(1);
					plane->Prepare();
					m_meshResourceMap[L"GeneratePlane"] = plane;
					return plane;
				}
				else {
					Mesh* newMesh = new Mesh(2);
					FbxReaderSG::Read(L"", newMesh);
					newMesh->m_material.ambient = XMFLOAT4(0.2, 0.2, 0.2, 1);
					newMesh->m_material.diffuse = XMFLOAT4(1.0, 0.5, 0.3, 1);
					newMesh->m_material.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 10.0f);

					newMesh->Prepare();
					// add to dictionary here

					

					return newMesh;
				}
			}
			else {
				return got->second;
			}
		}
	}

	~ResourceManager();

private:
	static ResourceManager* _instance;
	std::unordered_map<std::wstring, Mesh*> m_meshResourceMap;
};


#endif // !__RESOURCE_MANAGER__
