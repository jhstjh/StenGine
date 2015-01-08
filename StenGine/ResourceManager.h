#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__

#include <unordered_map>
#include "D3DIncludes.h"
#include "MeshRenderer.h"
#include <type_traits>
#include "FbxReaderSG.h"
#include "GL/glew.h"
#include "SOIL.h"
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
#ifdef GRAPHICS_D3D11
		else if (std::is_same<T, ID3D11ShaderResourceView>::value) {
			auto got = m_textureSRVResourceMap.find(path);
			if (got == m_textureSRVResourceMap.end()) {
				ID3D11ShaderResourceView* texSRV;
				CreateDDSTextureFromFile(D3D11Renderer::Instance()->GetD3DDevice(),
					path.c_str(), nullptr, &texSRV);
				m_textureSRVResourceMap[path] = texSRV;
				return (T*)texSRV;
			}
			else {
				return (T*)got->second;
			}
		}
#else
		else if (std::is_same<T, GLuint>::value) {
			auto got = m_textureResourceMap.find(path);
			if (got == m_textureResourceMap.end()) {
				std::string s(path.begin(), path.end());
				GLuint tex = SOIL_load_OGL_texture(
					s.c_str(),
					SOIL_LOAD_AUTO,
					SOIL_CREATE_NEW_ID,
					SOIL_FLAG_MIPMAPS | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_TEXTURE_RECTANGLE
					);
				assert(tex != 0);
				m_textureResourceMap[path] = tex;
				return (T*)&m_textureResourceMap[path];
			}
			else {
				return (T*)&(got->second);
			}
		}
#endif
	}

	~ResourceManager();

private:
	static ResourceManager* _instance;
	std::unordered_map<std::wstring, Mesh*> m_meshResourceMap;
#ifdef GRAPHICS_D3D11
	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> m_textureSRVResourceMap;
#else
	std::unordered_map<std::wstring, GLuint> m_textureResourceMap;
#endif
};


#endif // !__RESOURCE_MANAGER__
