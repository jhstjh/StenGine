#ifndef __EFFECTS_MANAGER__
#define __EFFECTS_MANAGER__

#include "System/API/PlatformAPIDefs.h"

#include "Scene/LightManager.h"
#include "Graphics/Abstraction/ConstantBuffer.h"
#include "Graphics/Effect/Material.h"
#include "Mesh/MeshRenderer.h"
#include "System/SingletonClass.h"
#include "System/AlignedClass.h"

#include <memory>

#if PLATFORM_WIN32

#if GRAPHICS_OPENGL
#include "Graphics/OpenGL/GLBuffer.h"
#include "glew.h"
#endif

#include "Graphics/D3DIncludes.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)
#elif  PLATFORM_ANDROID
#include <EGL/egl.h>
#include <GLES/gl.h>
#include "NDKHelper.h"
using namespace ndk_helper;
#endif



#pragma warning(disable: 4312)

namespace StenGine
{

class Mesh;

class Effect : public AlignedClass<16> {
protected:
#if PLATFORM_WIN32
#if GRAPHICS_D3D11
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11HullShader* m_hullShader;
	ID3D11DomainShader* m_domainShader;
	ID3D11ComputeShader* m_computeShader;

	ID3DBlob *m_vsBlob;
	ID3DBlob *m_psBlob;
	ID3DBlob *m_gsBlob;
	ID3DBlob *m_hsBlob;
	ID3DBlob *m_dsBlob;
	ID3DBlob *m_csBlob;

	ID3D11InputLayout* m_inputLayout;
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_vertexDesc;
	ID3D11ShaderResourceView** m_shaderResources;
	ID3D11ShaderResourceView** m_outputShaderResources;
	ID3D11UnorderedAccessView** m_unorderedAccessViews;

	void ReadShaderFile(std::wstring filename, ID3DBlob **blob, char* target, char* entryPoint = "main");
#else
	// gl effect
	GLuint m_vertexShader;
	GLuint m_pixelShader;
	GLuint m_geometryShader;
	GLuint m_hullShader;
	GLuint m_domainShader;
	GLuint m_computeShader;

	GLuint m_inputLayout;

	GLuint m_shaderProgram;
	bool ReadShaderFile(std::wstring filename, char* shaderContent, int maxLength);

#endif
public:
	Effect(const std::wstring& filename);
	Effect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath,
		const std::wstring& csPath);
	virtual ~Effect();
	virtual void SetShader();
	virtual void UpdateConstantBuffer() {};
	virtual void BindConstantBuffer() {};
	virtual void BindShaderResource() {}
	virtual void UnBindConstantBuffer();
	virtual void UnBindShaderResource();
	virtual void UnSetShader();

	void* GetInputLayout()
	{
		return (void*)m_inputLayout;
	}
#if GRAPHICS_D3D11
	virtual void UnbindUnorderedAccessViews();
	virtual void SetShaderResources(ID3D11ShaderResourceView* res, int idx);
	virtual ID3D11ShaderResourceView* GetOutputShaderResource(int idx);
	//virtual void GetUAVResources(ID3D11UnorderedAccessView* res, int idx) {}
#else
	// gl effect
#endif

#else
protected:
	GLuint m_vertexShader;
	GLuint m_pixelShader;

	GLuint m_shaderProgram;
public:
	Effect(const std::string vsPath, std::string psPath);

	virtual void SetShader();
	virtual void UpdateConstantBuffer() = 0;
	virtual void BindConstantBuffer() = 0;
	//virtual void BindShaderResource() {}
	//virtual void UnBindConstantBuffer();
	//virtual void UnBindShaderResource();
	//virtual void UnSetShader();
#endif

	std::vector<Mesh*> m_associatedMeshes;
};

#if PLATFORM_WIN32

class DeferredGeometryPassEffect : public Effect {

public:
	DeferredGeometryPassEffect(const std::wstring& filename);
	DeferredGeometryPassEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DeferredGeometryPassEffect();

	void PrepareBuffer();

#if GRAPHICS_D3D11
	struct PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX WorldViewInvTranspose;
		XMMATRIX WorldInvTranspose;
		XMMATRIX WorldView;
		XMMATRIX World;
		XMMATRIX ViewProj;
		XMMATRIX ShadowTransform;
		Material Mat;
		XMFLOAT4 DiffX_NormY_ShadZ;
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		XMFLOAT4 EyePosW;
	};

	ID3D11Buffer* m_perFrameCB;
	ID3D11Buffer* m_perObjectCB;
#else
	struct PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX World;
		XMMATRIX WorldView;
		XMMATRIX ViewProj;
		XMMATRIX ShadowTransform;
		Material Mat;
		XMFLOAT4 DiffX_NormY_ShadZ;
		uint64_t DiffuseMap;
		uint64_t NormalMap;
		uint64_t ShadowMapTex;
		uint64_t BumpMapTex;
		uint64_t CubeMapTex;
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		XMFLOAT4 EyePosW;
		DirectionalLight DirLight;
	};

	GLuint m_perFrameCB;
	GLuint m_perObjectCB;
#endif
};


//--------------------------------------------------------------------//


class DeferredGeometryTerrainPassEffect : public Effect {
public:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_perFrameCB;
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perFrameCB;
	GLuint m_perObjectCB;
#endif

	DeferredGeometryTerrainPassEffect(const std::wstring& filename);
	DeferredGeometryTerrainPassEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DeferredGeometryTerrainPassEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX WorldViewInvTranspose;
		XMMATRIX WorldInvTranspose;
		XMMATRIX WorldView;
		XMMATRIX World;
		XMMATRIX ViewProj;
		XMMATRIX ShadowTransform;
		XMMATRIX View;
		Material Mat;
		XMFLOAT4 DiffX_NormY_ShadZ;

#if GRAPHICS_OPENGL
		uint64_t gShadowMap;
		uint64_t gCubeMap;
		uint64_t gHeightMap;
		uint64_t gLayerMapArray;
		uint64_t gBlendMap;
#endif
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		XMFLOAT4 gEyePosW;

		// When distance is minimum, the tessellation is maximum.
		// When distance is maximum, the tessellation is minimum.
		float gMinDist;
		float gMaxDist;

		// Exponents for power of 2 tessellation.  The tessellation
		// range is [2^(gMinTess), 2^(gMaxTess)].  Since the maximum
		// tessellation is 64, this means gMaxTess can be at most 6
		// since 2^6 = 64.
		float gMinTess;
		float gMaxTess;

		float gTexelCellSpaceU;
		float gTexelCellSpaceV;
		float gWorldCellSpace;

		float pad1;
		XMFLOAT2 gTexScale;

		XMFLOAT2 pad2;
		XMFLOAT4 gWorldFrustumPlanes[6];
	};
};

//--------------------------------------------------------------------//


class TerrainShadowMapEffect : public Effect {
public:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_perFrameCB;
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perFrameCB;
	GLuint m_perObjectCB;
#endif

	TerrainShadowMapEffect(const std::wstring& filename);
	TerrainShadowMapEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~TerrainShadowMapEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX WorldViewInvTranspose;
		XMMATRIX WorldInvTranspose;
		XMMATRIX WorldView;
		XMMATRIX World;
		XMMATRIX ViewProj;
		XMMATRIX ShadowTransform;
		XMMATRIX View;
		Material Mat;
		XMFLOAT4 DiffX_NormY_ShadZ;

#if GRAPHICS_OPENGL
		uint64_t gShadowMap;
		uint64_t gCubeMap;
		uint64_t gHeightMap;
		uint64_t gLayerMapArray;
		uint64_t gBlendMap;
#endif
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		XMFLOAT4 gEyePosW;

		// When distance is minimum, the tessellation is maximum.
		// When distance is maximum, the tessellation is minimum.
		float gMinDist;
		float gMaxDist;

		// Exponents for power of 2 tessellation.  The tessellation
		// range is [2^(gMinTess), 2^(gMaxTess)].  Since the maximum
		// tessellation is 64, this means gMaxTess can be at most 6
		// since 2^6 = 64.
		float gMinTess;
		float gMaxTess;

		float gTexelCellSpaceU;
		float gTexelCellSpaceV;
		float gWorldCellSpace;

		float pad1;
		XMFLOAT2 gTexScale;

		XMFLOAT2 pad2;
		XMFLOAT4 gWorldFrustumPlanes[6];
	};
};

//--------------------------------------------------------------------//

class DeferredGeometrySkinnedPassEffect : public Effect {
private:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_perFrameCB;
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perFrameUBO;
	GLuint m_perObjectUBO;

	GLint DiffuseMapPosition;
	GLint NormalMapPosition;
	GLint ShadowMapPosition;
	GLint CubeMapPosition;
#endif

public:
	DeferredGeometrySkinnedPassEffect(const std::wstring& filename);
	DeferredGeometrySkinnedPassEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DeferredGeometrySkinnedPassEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();
#if GRAPHICS_D3D11
	struct PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX WorldViewInvTranspose;
		XMMATRIX WorldInvTranspose;
		XMMATRIX WorldView;
		XMMATRIX World;
		XMMATRIX ViewProj;
		XMMATRIX ShadowTransform;
		Material Mat;
		XMFLOAT4 DiffX_NormY_ShadZ;
	} m_perObjConstantBuffer;

	struct PERFRAME_CONSTANT_BUFFER
	{
		XMFLOAT4 EyePosW;
	} m_perFrameConstantBuffer;

	virtual PEROBJ_CONSTANT_BUFFER* GetPerObjConstantBuffer() { return &m_perObjConstantBuffer; }
	virtual PERFRAME_CONSTANT_BUFFER* GetPerFrameConstantBuffer() { return &m_perFrameConstantBuffer; }
	//ID3D11ShaderResourceView *m_shaderResources[5];
#else
	struct PEROBJ_UNIFORM_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX World;
		XMMATRIX WorldView;
		XMMATRIX ShadowTransform;
		Material Mat;
		XMFLOAT4 DiffX_NormY_ShadZ;
	} m_perObjUniformBuffer;

	struct PERFRAME_UNIFORM_BUFFER
	{
		XMFLOAT4 EyePosW;
		DirectionalLight DirLight;
	} m_perFrameUniformBuffer;

	GLint DiffuseMap;
	GLint NormalMap;
	GLint ShadowMapTex;
	GLint CubeMapTex;

#endif
};
//--------------------------------------------------------------------//


class DeferredGeometryTessPassEffect : public DeferredGeometryPassEffect {
private:


public:
	DeferredGeometryTessPassEffect(const std::wstring& filename);
	~DeferredGeometryTessPassEffect();
};


//--------------------------------------------------------------------//

class DeferredShadingPassEffect : public Effect {
public:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_perFrameCB;
#else
	GLuint m_perFrameCB;
#endif

	DeferredShadingPassEffect(const std::wstring& filename);
	~DeferredShadingPassEffect();

	struct
#if GRAPHICS_D3D11		
		PERFRAME_CONSTANT_BUFFER
	{
		DirectionalLight gDirLight;
		XMFLOAT4 gEyePosV;
		XMMATRIX gProjInv;
		XMMATRIX gProj;
	};
#endif

#if GRAPHICS_OPENGL
	PERFRAME_CONSTANT_BUFFER
	{
		DirectionalLight gDirLight;
		XMFLOAT4 gEyePosV;
		XMMATRIX gProjInv;
		XMMATRIX gProj;
		uint64_t NormalGMap;
		uint64_t DiffuseGMap;
		uint64_t SpecularGMap;
		uint64_t DepthGMap;
		uint64_t RandVectMap;
	};
#endif
};


//--------------------------------------------------------------------//


class ShadowMapEffect : public Effect {
private:


public:
	ShadowMapEffect(const std::wstring& filename);
	~ShadowMapEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX gWorldViewProj;
	};

#if GRAPHICS_D3D11
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perObjectCB;
#endif
};


//--------------------------------------------------------------------//


class GodRayEffect : public Effect {
private:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_perFrameCB;
#else
	GLuint m_perFrameUBO;
	GLint OcclusionMapPosition;
#endif

public:
	GodRayEffect(const std::wstring& filename);
	~GodRayEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();

	struct
#if GRAPHICS_D3D11		
		PERFRAME_CONSTANT_BUFFER
#else
		PERFRAME_UNIFORM_BUFFER
#endif
	{
		XMFLOAT4 gLightPosH;
	}
#if GRAPHICS_D3D11		
	m_perFrameConstantBuffer;
#else
	m_perFrameUniformBuffer;
#endif

#if GRAPHICS_OPENGL
	GLuint OcclusionMap;
#endif


};


//--------------------------------------------------------------------//


class SkyboxEffect : public Effect {
public:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perObjectCB;
#endif

public:
	SkyboxEffect(const std::wstring& filename);
	~SkyboxEffect();

	struct
#if GRAPHICS_D3D11
		PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX gWorldViewProj;
	};
#endif

#if GRAPHICS_OPENGL
	PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX gWorldViewProj;
		uint64_t gCubeMap;
	};
#endif
	//ID3D11ShaderResourceView *m_shaderResources[1];
};


//--------------------------------------------------------------------//


class BlurEffect : public Effect {
private:
	ID3D11Buffer* m_settingCB;

public:
	BlurEffect(const std::wstring& filename);
	~BlurEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();

	struct SETTING_CONSTANT_BUFFER
	{
		XMFLOAT2 texOffset;
		XMFLOAT2 pad;
	} m_settingConstantBuffer;

	/// Texture2D gScreenMap;
	/// Texture2D gSSAOMap;
	//ID3D11ShaderResourceView *m_shaderResources[2];
};


//--------------------------------------------------------------------//


class VBlurEffect : public Effect {
private:
	ID3D11Buffer* m_settingCB;

public:
	VBlurEffect(const std::wstring& filename);
	~VBlurEffect();

	struct SETTING_CONSTANT_BUFFER
	{
		XMFLOAT2 texOffset;
		XMFLOAT2 pad;
	};
};


//--------------------------------------------------------------------//


class HBlurEffect : public Effect {
private:
	ID3D11Buffer* m_settingCB;

public:
	HBlurEffect(const std::wstring& filename);
	~HBlurEffect();

	struct SETTING_CONSTANT_BUFFER
	{
		XMFLOAT2 texOffset;
		XMFLOAT2 pad;
	};
};


//--------------------------------------------------------------------//


class DeferredShadingCS : public Effect {
private:
	ID3D11Buffer* m_perFrameCB;

public:
	DeferredShadingCS(const std::wstring& filename);
	~DeferredShadingCS();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource(int idx = 0);

	struct PERFRAME_CONSTANT_BUFFER
	{
		DirectionalLight gDirLight;
		XMFLOAT4 gEyePosW;
		XMMATRIX gProjInv;
		XMMATRIX gProj;
	} m_perFrameConstantBuffer;

	/// Texture Ordering:
	/// Texture2D gDiffuseGB;
	/// Texture2D gNormalGB;
	/// Texture2D gSpecularGB;
	/// Texture2D gDepthGB;
	//ID3D11ShaderResourceView *m_shaderResources[4];
};

//--------------------------------------------------------------------//


class DebugLineEffect : public Effect {
private:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perObjectUBO;
#endif

public:
	DebugLineEffect(const std::wstring& filename);
	DebugLineEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DebugLineEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();

	struct
#if GRAPHICS_D3D11
		PEROBJ_CONSTANT_BUFFER
#else
		PEROBJ_UNIFORM_BUFFER
#endif
	{
		XMMATRIX ViewProj;
	}
#if GRAPHICS_D3D11
	m_perObjConstantBuffer;
#else
	m_perObjUniformBuffer;
#endif

#if GRAPHICS_D3D11
	virtual PEROBJ_CONSTANT_BUFFER* GetPerObjConstantBuffer() { return &m_perObjConstantBuffer; }
#endif
	//ID3D11ShaderResourceView *m_shaderResources[5];
};


/**************************************************************/

#else

class SimpleMeshEffect : public Effect {
private:
	GLuint m_perFrameUBO;
	GLuint m_perObjectUBO;

	//GLint DiffuseMapPosition;
	//GLint NormalMapPosition;
	//GLint ShadowMapPosition;
	//GLint CubeMapPosition;

public:
	SimpleMeshEffect(const std::string& filename);
	SimpleMeshEffect(const std::string& vsPath,
		const std::string& psPath);
	~SimpleMeshEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	//virtual void BindShaderResource();

	struct PEROBJ_UNIFORM_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX World;
		XMMATRIX WorldView;
		XMMATRIX ShadowTransform;
		Material Mat;
		XMFLOAT4 DiffX_NormY_ShadZ;
	} m_perObjUniformBuffer;

	struct PERFRAME_UNIFORM_BUFFER
	{
		XMFLOAT4 EyePosW;
		DirectionalLight DirLight;
	} m_perFrameUniformBuffer;

	//GLint DiffuseMap;
	//GLint NormalMap;
	//GLint ShadowMapTex;
	//GLint CubeMapTex;
};

class DebugLineEffect : public Effect {
private:
	GLuint m_perObjectUBO;

public:
	DebugLineEffect(const std::string &filename);
	DebugLineEffect(const char* vsPath, const char* psPath);
	~DebugLineEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();

	struct PEROBJ_UNIFORM_BUFFER
	{
		Mat4 ViewProj;
	}
	m_perObjUniformBuffer;
};


#endif

class EffectsManager : public SingletonClass<EffectsManager> {
public:
	EffectsManager();
	~EffectsManager();

#if PLATFORM_WIN32
	//StdMeshEffect* m_stdMeshEffect;
	std::unique_ptr<ShadowMapEffect> m_shadowMapEffect;
	std::unique_ptr<TerrainShadowMapEffect> m_terrainShadowMapEffect;
	std::unique_ptr<DeferredGeometryPassEffect> m_deferredGeometryPassEffect;
	std::unique_ptr<DeferredGeometrySkinnedPassEffect> m_deferredGeometrySkinnedPassEffect;
	std::unique_ptr<DeferredGeometryTerrainPassEffect> m_deferredGeometryTerrainPassEffect;
	std::unique_ptr<DeferredGeometryTessPassEffect> m_deferredGeometryTessPassEffect;
	std::unique_ptr<DeferredShadingPassEffect> m_deferredShadingPassEffect;
	std::unique_ptr<DeferredShadingCS> m_deferredShadingCSEffect;
	std::unique_ptr<GodRayEffect> m_godrayEffect;
	std::unique_ptr<SkyboxEffect> m_skyboxEffect;
	std::unique_ptr<BlurEffect> m_blurEffect;
	std::unique_ptr<VBlurEffect> m_vblurEffect;
	std::unique_ptr<HBlurEffect> m_hblurEffect;
	std::unique_ptr<DebugLineEffect> m_debugLineEffect;
#else
	DebugLineEffect* m_debugLineEffect;
	SimpleMeshEffect* m_simpleMeshEffect;
#endif // !PLATFORM_ANDROID
};

}
#endif