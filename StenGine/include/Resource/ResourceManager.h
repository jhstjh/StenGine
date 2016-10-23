#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__

#include <unordered_map>
#include "Graphics/Abstraction/RendererBase.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/SkinnedMesh.h"
#include "System/SingletonClass.h"
#include "Graphics/Abstraction/Texture.h"
#include "Graphics/Animation/Animation.h"

#include "Graphics/OpenGL/GLImageLoader.h"
#include "glew.h"


#include "Graphics/D3DIncludes.h"
#include <type_traits>
#include "Utility/FbxReaderSG.h"


namespace StenGine
{

class ResourceManager : public SingletonClass<ResourceManager> {
public:
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
	T* GetResource(std::wstring path) 
	{
		return nullptr;
	}

	template <>
	Mesh* GetResource<Mesh>(std::wstring path) 
	{
		auto got = m_meshResourceMap.find(path);
		if (got == m_meshResourceMap.end()) 
		{
			if (path == L"GenerateBox") 
			{
				Mesh* box = new Mesh(0);
				box->Prepare();
				m_meshResourceMap[L"GenerateBox"] = box;
				return box;
			}
			else if (path == L"GeneratePlane") 
			{
				Mesh* plane = new Mesh(1);
				plane->Prepare();
				m_meshResourceMap[L"GeneratePlane"] = plane;
				return plane;
			}
			else 
			{
				Mesh* newMesh = new Mesh(2);
				bool result = FbxReaderSG::Read(path, newMesh);
				assert(result);
				newMesh->Prepare();
				m_meshResourceMap[path] = newMesh;

				return newMesh;
			}
		}
		else 
		{
			return got->second;
		}
	}

	template <>
	SkinnedMesh* GetResource<SkinnedMesh>(std::wstring path)
	{
		auto got = m_meshResourceMap.find(path);
		if (got == m_meshResourceMap.end()) 
		{
			SkinnedMesh* newMesh = new SkinnedMesh();
			bool result = FbxReaderSG::Read(path, newMesh);
			assert(result);
			newMesh->Prepare();
			m_meshResourceMap[path] = newMesh;

			return newMesh;
		}
		else 
		{
			return dynamic_cast<SkinnedMesh*>(got->second);
		}
	}

	template <>
	Animation* GetResource<Animation>(std::wstring path)
	{
		auto got = m_animationResourceMap.find(path);
		if (got == m_animationResourceMap.end())
		{
			Animation* newAnimation = new Animation();
			bool result = FbxReaderSG::Read(path, newAnimation);
			assert(result);

			m_animationResourceMap[path] = newAnimation;

			return newAnimation;
		}
		else
		{
			return dynamic_cast<Animation*>(got->second);
		}
	}

	template <>
	Texture* GetResource<Texture>(std::wstring path) 
	{
		switch (Renderer::GetRenderBackend())
		{
		case RenderBackend::D3D11:
		{
			auto got = m_textureResourceMap.find(path);
			if (got == m_textureResourceMap.end())
			{
				ID3D11ShaderResourceView* texSRV;
				ID3D11Texture2D* texRes;
				HR(CreateDDSTextureFromFile(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice()),
					path.c_str(), (ID3D11Resource**)(&texRes), &texSRV));

				D3D11_TEXTURE2D_DESC texElementDesc;
				texRes->GetDesc(&texElementDesc);

				m_textureResourceMap[path] = new Texture(texElementDesc.Width, texElementDesc.Height, texSRV);
			}
			return m_textureResourceMap[path];
		}
		case RenderBackend::OPENGL4:
		{
			auto got = m_textureResourceMap.find(path);
			if (got == m_textureResourceMap.end()) {
				std::string s(path.begin(), path.end());
				uint32_t width, height;
				GLuint tex = CreateGLTextureFromFile(s.c_str(), &width, &height);
				assert(tex != 0);

				m_textureResourceMap[path] = new Texture(width, height, reinterpret_cast<void*>(tex));
			}
			return m_textureResourceMap[path];
		}
		}
		return nullptr;
	}

	// cubemap
	template <typename T>
	T* GetResource(std::vector<std::wstring> &filenames) {
		switch (Renderer::GetRenderBackend())
		{
		case RenderBackend::D3D11:
		{
			if (std::is_same<T, Texture>::value) {
				//
				// Load the texture elements individually from file.  These textures
				// won't be used by the GPU (0 bind flags), they are just used to 
				// load the image data from file.  We use the STAGING usage so the
				// CPU can read the resource.
				//
				UINT size = (UINT)filenames.size();
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

				return new Texture(texArrayDesc.Width, texArrayDesc.Height, textureArraySRV);
			}
		}
		case RenderBackend::OPENGL4:
		{
			if (std::is_same<T, Texture>::value) {
				std::wstring paths;

				for (auto &filename : filenames)
				{
					paths += filename;
				}

				auto got = m_textureResourceMap.find(paths);
				if (got == m_textureResourceMap.end()) {
					uint32_t width, height;
					GLuint tex = CreateGLTextureArrayFromFiles(filenames, &width, &height);
					assert(tex != 0);

					glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

					m_textureResourceMap[paths] = new Texture(width, height, reinterpret_cast<void*>(tex));
				}
				return m_textureResourceMap[paths];
			}
		}
		}

		return nullptr;
	}

	~ResourceManager();

private:
	std::unordered_map<std::wstring, Mesh*> m_meshResourceMap;
	std::unordered_map<std::wstring, Texture*> m_textureResourceMap;
	std::unordered_map<std::wstring, Animation*> m_animationResourceMap;
};

}
#endif // !__RESOURCE_MANAGER__
