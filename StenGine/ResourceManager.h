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
	T* GetResource(const char* path) {
		std::string s(path);
		std::wstring ws(s.begin(), s.end());
		return GetResource<T>(ws);
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
					return (T*)box;
				}
				else if (path == L"GeneratePlane") {
					Mesh* plane = new Mesh(1);
					plane->Prepare();
					m_meshResourceMap[L"GeneratePlane"] = plane;
					return (T*)plane;
				}
				else {
					Mesh* newMesh = new Mesh(2);
					bool result = FbxReaderSG::Read(path, newMesh);
					assert(result);
					newMesh->Prepare();
					m_meshResourceMap[path] = newMesh;
					
					return (T*)newMesh;
				}
			}
			else {
				return (T*)got->second;
			}
		}
		else if (std::is_same<T, ID3D11ShaderResourceView>::value) {
			auto got = m_textureSRVResourceMap.find(path);
			if (got == m_textureSRVResourceMap.end()) {
				ID3D11ShaderResourceView* texSRV;
				CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
					path.c_str(), nullptr, &texSRV);
				return (T*)texSRV;
			}
			else {
				return (T*)got->second;
			}
		}
	}

	~ResourceManager();

private:
	static ResourceManager* _instance;
	std::unordered_map<std::wstring, Mesh*> m_meshResourceMap;
	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> m_textureSRVResourceMap;
};


#endif // !__RESOURCE_MANAGER__
