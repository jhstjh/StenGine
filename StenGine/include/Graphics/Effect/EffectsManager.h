#ifndef __EFFECTS_MANAGER__
#define __EFFECTS_MANAGER__

#include "Graphics/Abstraction/RendererBase.h"
#include "System/API/PlatformAPIDefs.h"

#include "Scene/LightManager.h"
#include "Graphics/Abstraction/ConstantBuffer.h"
#include "Graphics/Abstraction/GPUBuffer.h"
#include "Graphics/Effect/Material.h"
#include "Math/MathDefs.h"
#include "Mesh/MeshRenderer.h"
#include "System/SingletonClass.h"
#include "System/AlignedClass.h"

#include <memory>

#include "Graphics/OpenGL/GLBuffer.h"
#include "glew.h"

#include "Graphics/D3DIncludes.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

#pragma warning(disable: 4312)

namespace StenGine
{

class Mesh;
class Renderer;

class Effect : public AlignedClass<16> {
protected:

	/****TODO this is quite ugly here****/
	/****either create a shader abstraction or Impl this****/
	ID3D11VertexShader* m_d3d11vertexShader;
	ID3D11PixelShader* m_d3d11pixelShader;
	ID3D11GeometryShader* m_d3d11geometryShader;
	ID3D11HullShader* m_d3d11hullShader;
	ID3D11DomainShader* m_d3d11domainShader;
	ID3D11ComputeShader* m_d3d11computeShader;

	ID3DBlob *m_vsBlob;
	ID3DBlob *m_psBlob;
	ID3DBlob *m_gsBlob;
	ID3DBlob *m_hsBlob;
	ID3DBlob *m_dsBlob;
	ID3DBlob *m_csBlob;

	ID3D11InputLayout* m_d3d11inputLayout;
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_vertexDesc;
	ID3D11ShaderResourceView** m_shaderResources;
	ID3D11ShaderResourceView** m_outputShaderResources;
	ID3D11UnorderedAccessView** m_unorderedAccessViews;

	void ReadShaderFile(std::wstring filename, ID3DBlob **blob, char* target, char* entryPoint = "main");

	// gl effect
	GLuint m_glvertexShader;
	GLuint m_glpixelShader;
	GLuint m_glgeometryShader;
	GLuint m_glhullShader;
	GLuint m_gldomainShader;
	GLuint m_glcomputeShader;

	GLuint m_glinputLayout;

	GLuint m_shaderProgram;
	bool ReadShaderFile(std::wstring filename, char* shaderContent, int maxLength);
	/*******************************************************/

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

	void* GetInputLayout();

	// TODO D3D specific
	virtual void UnbindUnorderedAccessViews();
	virtual void SetShaderResources(ID3D11ShaderResourceView* res, int idx);
	virtual ID3D11ShaderResourceView* GetOutputShaderResource(int idx);
	//virtual void GetUAVResources(ID3D11UnorderedAccessView* res, int idx) {}

	std::vector<Mesh*> m_associatedMeshes;
};

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

	struct PEROBJ_CONSTANT_BUFFER
	{
		Mat4 WorldViewProj;
		Mat4 WorldViewInvTranspose;
		Mat4 WorldInvTranspose;
		Mat4 WorldView;
		Mat4 World;
		Mat4 ViewProj;
		Mat4 ShadowTransform;
		Material::MaterialAttrib Mat;
		Vec4 DiffX_NormY_ShadZ;
	};

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t DiffuseMap;
		uint64_t NormalMap;
		uint64_t ShadowMapTex;
		uint64_t BumpMapTex;
		uint64_t CubeMapTex;
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		Vec4 EyePosW;
		DirectionalLight DirLight;
	};

	GPUBuffer* m_perFrameCB;
	GPUBuffer* m_perObjectCB;
	GPUBuffer* m_textureCB;
};


//--------------------------------------------------------------------//

class DeferredSkinnedGeometryPassEffect : public Effect {

public:
	DeferredSkinnedGeometryPassEffect(const std::wstring& filename);
	DeferredSkinnedGeometryPassEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	virtual ~DeferredSkinnedGeometryPassEffect();

	void PrepareBuffer();

	struct PEROBJ_CONSTANT_BUFFER
	{
		Mat4 WorldViewProj;
		Mat4 WorldViewInvTranspose;
		Mat4 WorldInvTranspose;
		Mat4 WorldView;
		Mat4 World;
		Mat4 ViewProj;
		Mat4 ShadowTransform;
		Material::MaterialAttrib Mat;
		Vec4 DiffX_NormY_ShadZ;
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		Vec4 EyePosW;
	};

	struct MATRIX_PALETTE_BUFFER
	{
		Mat4 MatrixPalette[64];
	};

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t DiffuseMap;
		uint64_t NormalMap;
		uint64_t ShadowMapTex;
		uint64_t BumpMapTex;
		uint64_t CubeMapTex;
	};

	GPUBuffer* m_perFrameCB;
	GPUBuffer* m_perObjectCB;
	GPUBuffer* m_textureCB;
	GPUBuffer* m_matrixPaletteSB;
};


//--------------------------------------------------------------------//

class DeferredGeometryTerrainPassEffect : public Effect {
public:
	GPUBuffer* m_perFrameCB;
	GPUBuffer* m_perObjectCB;
	GPUBuffer* m_textureCB;

	DeferredGeometryTerrainPassEffect(const std::wstring& filename);
	DeferredGeometryTerrainPassEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DeferredGeometryTerrainPassEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		Mat4 WorldViewProj;
		Mat4 WorldViewInvTranspose;
		Mat4 WorldInvTranspose;
		Mat4 WorldView;
		Mat4 World;
		Mat4 ViewProj;
		Mat4 ShadowTransform;
		Mat4 View;
		Material::MaterialAttrib Mat;
		Vec4 DiffX_NormY_ShadZ;
	};

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t gShadowMap;
		uint64_t gCubeMap;
		uint64_t gHeightMap;
		uint64_t gLayerMapArray;
		uint64_t gBlendMap;
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		Vec4 gEyePosW;

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
		Vec2 gTexScale;

		Vec2 pad2;
		Vec4 gWorldFrustumPlanes[6];
	};
};

//--------------------------------------------------------------------//


class TerrainShadowMapEffect : public Effect {
public:
	GPUBuffer* m_perFrameCB;
	GPUBuffer* m_perObjectCB;
	GPUBuffer* m_textureCB;

	TerrainShadowMapEffect(const std::wstring& filename);
	TerrainShadowMapEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~TerrainShadowMapEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		Mat4 WorldViewProj;
		Mat4 WorldViewInvTranspose;
		Mat4 WorldInvTranspose;
		Mat4 WorldView;
		Mat4 World;
		Mat4 ViewProj;
		Mat4 ShadowTransform;
		Mat4 View;
		Material::MaterialAttrib Mat;
		Vec4 DiffX_NormY_ShadZ;
	};

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t gShadowMap;
		uint64_t gCubeMap;
		uint64_t gHeightMap;
		uint64_t gLayerMapArray;
		uint64_t gBlendMap;
	};

	struct PERFRAME_CONSTANT_BUFFER
	{
		Vec4 gEyePosW;

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
		Vec2 gTexScale;

		Vec2 pad2;
		Vec4 gWorldFrustumPlanes[6];
	};
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
	GPUBuffer* m_perFrameCB;
	GPUBuffer* m_textureCB;

	DeferredShadingPassEffect(const std::wstring& filename);
	~DeferredShadingPassEffect();

	struct PERFRAME_CONSTANT_BUFFER
	{
		DirectionalLight gDirLight;
		Vec4 gEyePosV;
		Mat4 gProjInv;
		Mat4 gProj;
	};

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t NormalGMap;
		uint64_t DiffuseGMap;
		uint64_t SpecularGMap;
		uint64_t DepthGMap;
		uint64_t RandVectMap;
	};

};


//--------------------------------------------------------------------//


class ShadowMapEffect : public Effect {
private:


public:
	ShadowMapEffect(const std::wstring& filename);
	~ShadowMapEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		Mat4 gWorldViewProj;
	};

	GPUBuffer* m_perObjectCB;
};


//--------------------------------------------------------------------//


class SkyboxEffect : public Effect {
public:

	GPUBuffer* m_perObjectCB;
	GPUBuffer* m_textureCB;

public:
	SkyboxEffect(const std::wstring& filename);
	~SkyboxEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		Mat4 gWorldViewProj;
	};

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t gCubeMap;
	};
};


//--------------------------------------------------------------------//


class BlurEffect : public Effect {
public:

	GPUBuffer* m_settingCB;
	GPUBuffer* m_textureCB;

public:
	BlurEffect(const std::wstring& filename);
	~BlurEffect();

	struct SETTING_CONSTANT_BUFFER
	{
		Vec2 texOffset;
		Vec2 xEnableSSAO;
	} ;

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t ScreenMap;
		uint64_t SSAOMap;
		uint64_t BloomMap;
		uint64_t DepthMap;
	};
};


//--------------------------------------------------------------------//


class VBlurEffect : public Effect {

public:
	VBlurEffect(const std::wstring& filename);
};


//--------------------------------------------------------------------//


class HBlurEffect : public Effect {

public:
	HBlurEffect(const std::wstring& filename);
};

//--------------------------------------------------------------------//


class ImGuiEffect : public Effect {
public:
	
	GPUBuffer* m_imguiCB;
	GPUBuffer* m_textureCB;

	ImGuiEffect(const std::wstring& filename);
	~ImGuiEffect();

	struct IMGUI_CONSTANT_BUFFER
	{
		Mat4 ProjMtx;
	};

	struct BINDLESS_TEXTURE_CONSTANT_BUFFER
	{
		uint64_t Texture;
	};
};


//--------------------------------------------------------------------//


class DebugLineEffect : public Effect {
public:
	GPUBuffer* m_perObjectCB;

	DebugLineEffect(const std::wstring& filename);
	DebugLineEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DebugLineEffect();

	struct PEROBJ_CONSTANT_BUFFER
	{
		Mat4 ViewProj;
	};
};


/**************************************************************/



class EffectsManager : public SingletonClass<EffectsManager> {
public:
	EffectsManager();
	~EffectsManager();

	//StdMeshEffect* m_stdMeshEffect;
	std::unique_ptr<ShadowMapEffect> m_shadowMapEffect;
	std::unique_ptr<TerrainShadowMapEffect> m_terrainShadowMapEffect;
	std::unique_ptr<DeferredGeometryPassEffect> m_deferredGeometryPassEffect;
	std::unique_ptr<DeferredSkinnedGeometryPassEffect> m_deferredSkinnedGeometryPassEffect;
	std::unique_ptr<DeferredGeometryTerrainPassEffect> m_deferredGeometryTerrainPassEffect;
	std::unique_ptr<DeferredGeometryTessPassEffect> m_deferredGeometryTessPassEffect;
	std::unique_ptr<DeferredShadingPassEffect> m_deferredShadingPassEffect;
	std::unique_ptr<SkyboxEffect> m_skyboxEffect;
	std::unique_ptr<BlurEffect> m_blurEffect;
	std::unique_ptr<VBlurEffect> m_vblurEffect;
	std::unique_ptr<HBlurEffect> m_hblurEffect;
	std::unique_ptr<DebugLineEffect> m_debugLineEffect;
	std::unique_ptr<ImGuiEffect> m_imguiEffect;
};

}
#endif