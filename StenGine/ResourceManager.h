#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__

#include <unordered_map>
#include "MeshRenderer.h"
//#include "SOIL.h"

#ifdef GRAPHICS_OPENGL
#include "GLImageLoader.h"
#endif

#ifdef PLATFORM_WIN32
#include "D3DIncludes.h"
#include <type_traits>
#include "FbxReaderSG.h"
#include "GL/glew.h"
#elif defined PLATFORM_ANDROID
#include <assert.h>
#include "SgmReader.h"
#endif
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
	T* GetResource(std::string path) {
		std::wstring _path(path.begin(), path.end());
		if (std::is_same<T, Mesh>::value) {
			auto got = m_meshResourceMap.find(_path);
			if (got == m_meshResourceMap.end()) {

				Mesh* newMesh = new Mesh(2);

				bool result = SgmReader::Read(path, newMesh);
				assert(result);
				newMesh->Prepare();
				m_meshResourceMap[_path] = newMesh;

				return (T*)newMesh;
			}
			else {
				return (T*)got->second;
			}
		}
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
#ifdef PLATFORM_WIN32
					Mesh* newMesh = new Mesh(2);
					bool result = FbxReaderSG::Read(path, newMesh);
					assert(result);
					newMesh->Prepare();
					m_meshResourceMap[path] = newMesh;
					
					return (T*)newMesh;
#elif defined(PLATFORM_ANDROID)
					Mesh* newMesh = new Mesh(2);

					std::string _path(path.begin(), path.end());
					bool result = SgmReader::Read(_path, newMesh);
					assert(result);
					newMesh->Prepare();
					m_meshResourceMap[path] = newMesh;

					return (T*)newMesh;
#endif
				}

			}
			else {
				return (T*)got->second;
			}
		}
//#ifdef PLATFORM_WIN32
#ifdef GRAPHICS_D3D11
		else if (std::is_same<T, ID3D11ShaderResourceView>::value) {
			auto got = m_textureSRVResourceMap.find(path);
			if (got == m_textureSRVResourceMap.end()) {
				ID3D11ShaderResourceView* texSRV;
				HR(CreateDDSTextureFromFile(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice()),
					path.c_str(), nullptr, &texSRV));
				m_textureSRVResourceMap[path] = texSRV;
				return (T*)texSRV;
			}
			else {
				return (T*)got->second;
			}
		}
#else
		else if (std::is_same<T, uint64_t>::value) {
			auto got = m_textureResourceMap.find(path);
			if (got == m_textureResourceMap.end()) {
				std::string s(path.begin(), path.end());
				uint64_t tex = CreateGLTextureFromFile(s.c_str());
				assert(tex != 0);
				glMakeTextureHandleResidentARB(tex);
				m_textureResourceMap[path] = tex;
				return (T*)&m_textureResourceMap[path];
			}
			else {
				return (T*)&(got->second);
			}
		}
#endif
//#endif
		return nullptr;
	}

	template <typename T>
	T* GetResource(std::vector<std::wstring> &filenames) {
#ifdef PLATFORM_WIN32
#ifdef GRAPHICS_D3D11
		if (std::is_same<T, ID3D11ShaderResourceView>::value) {
			//
			// Load the texture elements individually from file.  These textures
			// won't be used by the GPU (0 bind flags), they are just used to 
			// load the image data from file.  We use the STAGING usage so the
			// CPU can read the resource.
			//
			UINT size = filenames.size();
			std::vector<ID3D11Texture2D*> srcTex(size);
			auto device = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice());
			for (UINT i = 0; i < size; ++i)
			{
				HR(CreateDDSTextureFromFileEx(device, filenames[i].c_str(), 0u, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&srcTex[i], nullptr, nullptr));
			}

			//
			// Create the texture array.  Each element in the texture 
			// array has the same format/dimensions.
			//
			D3D11_TEXTURE2D_DESC texElementDesc;
			srcTex[0]->GetDesc(&texElementDesc);
			D3D11_TEXTURE2D_DESC texArrayDesc;
			texArrayDesc.Width = texElementDesc.Width;
			texArrayDesc.Height = texElementDesc.Height;
			texArrayDesc.MipLevels = texElementDesc.MipLevels;
			texArrayDesc.ArraySize = size;
			texArrayDesc.Format = texElementDesc.Format;
			texArrayDesc.SampleDesc.Count = 1;
			texArrayDesc.SampleDesc.Quality = 0;
			texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
			texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texArrayDesc.CPUAccessFlags = 0;
			texArrayDesc.MiscFlags = 0;
			ID3D11Texture2D* texArray = 0;
			HR(device->CreateTexture2D(&texArrayDesc, 0, &texArray));

			//
			// Copy individual texture elements into texture array.
			//
			auto context = static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext());

			// for each texture element...
			for (UINT texElement = 0; texElement < size; ++texElement)
			{
				// for each mipmap level...
				for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
				{
					D3D11_MAPPED_SUBRESOURCE mappedTex2D;
					HR(context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));
					context->UpdateSubresource(texArray,
						D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
						0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);
					context->Unmap(srcTex[texElement], mipLevel);
				}
			}
			//
			// Create a resource view to the texture array.
			//

			ID3D11ShaderResourceView* textureArraySRV;

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			viewDesc.Format = texArrayDesc.Format;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			viewDesc.Texture2DArray.ArraySize = size;
			HR(device->CreateShaderResourceView(texArray, &viewDesc, &textureArraySRV));

			return textureArraySRV;
		}
#else
		if (std::is_same<T, GLuint>::value) {
			auto got = m_textureResourceMap.find(path);
			if (got == m_textureResourceMap.end()) {
				std::string s(path.begin(), path.end());
				GLuint tex = CreateGLTextureFromFile(s.c_str());
				assert(tex != 0);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				m_textureResourceMap[path] = tex;
				return (T*)&m_textureResourceMap[path];
			}
			else {
				return (T*)&(got->second);
			}
		}
#endif
#endif
	}

	~ResourceManager();

private:
	static ResourceManager* _instance;
	std::unordered_map<std::wstring, Mesh*> m_meshResourceMap;
#ifdef GRAPHICS_D3D11
	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> m_textureSRVResourceMap;
#else
	std::unordered_map<std::wstring, uint64_t> m_textureResourceMap;
#endif
};


#endif // !__RESOURCE_MANAGER__
