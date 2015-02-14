#ifndef __EFFECTS_MANAGER__
#define __EFFECTS_MANAGER__
#include "D3DIncludes.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "LightManager.h"
#include "GL/glew.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

class Mesh;

class Effect {
protected:
#ifdef GRAPHICS_D3D11
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
	virtual void UpdateConstantBuffer() = 0;
	virtual void BindConstantBuffer() = 0;
	virtual void BindShaderResource() {} 
	virtual void UnBindConstantBuffer();
	virtual void UnBindShaderResource();
	virtual void UnSetShader();
#ifdef GRAPHICS_D3D11
	virtual void UnbindUnorderedAccessViews();
	virtual void SetShaderResources(ID3D11ShaderResourceView* res, int idx);
	virtual ID3D11ShaderResourceView* GetOutputShaderResource(int idx);
	//virtual void GetUAVResources(ID3D11UnorderedAccessView* res, int idx) {}
	ID3D11InputLayout* GetInputLayout() { return m_inputLayout; }
#else
	// gl effect
#endif
	std::vector<Mesh*> m_associatedMeshes;
};


//--------------------------------------------------------------------//


// class StdMeshEffect : public Effect {
// public:
// 	StdMeshEffect(const std::wstring& filename);
// 	~StdMeshEffect();
// 
// 	ID3DX11EffectTechnique* StdMeshTech;
// 	ID3DX11EffectMatrixVariable* WorldViewProj;
// 	ID3DX11EffectMatrixVariable* WorldInvTranspose;
// 	ID3DX11EffectMatrixVariable* World;
// 	ID3DX11EffectMatrixVariable* ShadowTransform;
// 	ID3DX11EffectVariable* DirLight;
// 	ID3DX11EffectVariable* Mat;
// 	ID3DX11EffectVectorVariable* EyePosW;
// 	ID3DX11EffectShaderResourceVariable* DiffuseMap;
// 	ID3DX11EffectShaderResourceVariable* TheShadowMap;
// };


//--------------------------------------------------------------------//


class DeferredGeometryPassEffect : public Effect {
private:
#ifdef GRAPHICS_D3D11
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
	DeferredGeometryPassEffect(const std::wstring& filename);
	DeferredGeometryPassEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DeferredGeometryPassEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();
#ifdef GRAPHICS_D3D11
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


class DeferredGeometryTerrainPassEffect : public Effect {
private:
#ifdef GRAPHICS_D3D11
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
	DeferredGeometryTerrainPassEffect(const std::wstring& filename);
	DeferredGeometryTerrainPassEffect(const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& gsPath,
		const std::wstring& hsPath,
		const std::wstring& dsPath);
	~DeferredGeometryTerrainPassEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();
#ifdef GRAPHICS_D3D11
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
	} m_perObjConstantBuffer;

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

class DeferredGeometrySkinnedPassEffect : public Effect {
private:
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
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
	ID3D11Buffer* m_perFrameCB;
	ID3D11Buffer* m_perObjectCB;

public:
	DeferredGeometryTessPassEffect(const std::wstring& filename);
	~DeferredGeometryTessPassEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();
};


//--------------------------------------------------------------------//

class DeferredShadingPassEffect : public Effect {
private:
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* m_perFrameCB;
#else
	GLint DiffuseGMapPosition;
	GLint NormalGMapPosition;
	GLint SpecularGMapPosition;
	GLint DepthGMapPosition;

	GLuint m_perFrameUBO;
#endif

public:
	DeferredShadingPassEffect(const std::wstring& filename);
	~DeferredShadingPassEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();

	struct 
#ifdef GRAPHICS_D3D11		
	PERFRAME_CONSTANT_BUFFER
#else
	PERFRAME_UNIFORM_BUFFER
#endif
	{
		DirectionalLight gDirLight;
		XMFLOAT4 gEyePosV;
		XMMATRIX gProjInv;
		XMMATRIX gProj;
	} 
#ifdef GRAPHICS_D3D11		
	m_perFrameConstantBuffer;
#else
	m_perFrameUniformBuffer;
#endif

#ifdef GRAPHICS_OPENGL
	GLuint DiffuseGMap;
	GLuint NormalGMap;
	GLuint SpecularGMap;
	GLuint DepthGMap;
#endif

	/// Texture Ordering:
	/// Texture2D gDiffuseGB;
	/// Texture2D gNormalGB;
	/// Texture2D gSpecularGB;
	/// Texture2D gDepthGB;
	//ID3D11ShaderResourceView *m_shaderResources[4];
};


//--------------------------------------------------------------------//


class ShadowMapEffect : public Effect {
private:
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perObjectUBO;
#endif

public:
	ShadowMapEffect(const std::wstring& filename);
	~ShadowMapEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	
	struct 
#ifdef GRAPHICS_D3D11
	PEROBJ_CONSTANT_BUFFER
#else
	PEROBJ_UNIFORM_BUFFER
#endif
	{
		XMMATRIX gWorldViewProj;
	} 
#ifdef GRAPHICS_D3D11
	m_perObjConstantBuffer;
#else
	m_perObjUniformBuffer;
#endif
};


//--------------------------------------------------------------------//


// class ScreenQuadEffect : public Effect {
// public:
// 	ScreenQuadEffect(const std::wstring& filename);
// 	~ScreenQuadEffect();
// 
// 	ID3DX11EffectTechnique* FullScreenQuadTech;
// 	ID3DX11EffectTechnique* DeferredLightingTech;
// 	ID3DX11EffectTechnique* HBlurTech;
// 	ID3DX11EffectTechnique* VBlurTech;
// 	ID3DX11EffectShaderResourceVariable* ScreenMap;
// 	ID3DX11EffectShaderResourceVariable* SSAOMap;
// 	ID3DX11EffectShaderResourceVariable* DiffuseGB;
// 	ID3DX11EffectShaderResourceVariable* PositionGB;
// 	ID3DX11EffectShaderResourceVariable* NormalGB;
// 	ID3DX11EffectShaderResourceVariable* SpecularGB;
// 	ID3DX11EffectShaderResourceVariable* DepthGB;
// 	ID3DX11EffectVariable* DirLight;
// 	//ID3DX11EffectVariable* Mat;
// 	ID3DX11EffectVectorVariable* EyePosW;
// 	ID3DX11EffectMatrixVariable* ProjInv;
// 	ID3DX11EffectMatrixVariable* Proj;
// };


//--------------------------------------------------------------------//


class GodRayEffect : public Effect {
private:
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11		
		PERFRAME_CONSTANT_BUFFER
#else
		PERFRAME_UNIFORM_BUFFER
#endif
	{
		XMFLOAT4 gLightPosH;
	}
#ifdef GRAPHICS_D3D11		
	m_perFrameConstantBuffer;
#else
	m_perFrameUniformBuffer;
#endif

#ifdef GRAPHICS_OPENGL
	GLuint OcclusionMap;
#endif


};


//--------------------------------------------------------------------//


class SkyboxEffect : public Effect {
public:
private:
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* m_perObjectCB;
#else
	GLuint m_perObjectUBO;
	GLint CubeMapPosition;
#endif

public:
	SkyboxEffect(const std::wstring& filename);
	~SkyboxEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();

	struct 
#ifdef GRAPHICS_D3D11
	PEROBJ_CONSTANT_BUFFER
#else
	PEROBJ_UNIFORM_BUFFER
#endif
	{
		XMMATRIX gWorldViewProj;
	} 
#ifdef GRAPHICS_D3D11
	m_perObjConstantBuffer;
#else
	m_perObjUniformBuffer;
#endif

	//ID3D11ShaderResourceView *m_shaderResources[1];
#ifdef GRAPHICS_OPENGL
	GLuint CubeMap;
#endif
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

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource(int idx = 0);

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


class HBlurEffect : public Effect {
private:
	ID3D11Buffer* m_settingCB;

public:
	HBlurEffect(const std::wstring& filename);
	~HBlurEffect();

	virtual void UpdateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource(int idx = 0);

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
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	PEROBJ_CONSTANT_BUFFER
#else
	PEROBJ_UNIFORM_BUFFER
#endif
	{
		XMMATRIX ViewProj;
	} 
#ifdef GRAPHICS_D3D11
	m_perObjConstantBuffer;
#else
	m_perObjUniformBuffer;
#endif

#ifdef GRAPHICS_D3D11
	virtual PEROBJ_CONSTANT_BUFFER* GetPerObjConstantBuffer() { return &m_perObjConstantBuffer; }
#endif
	//ID3D11ShaderResourceView *m_shaderResources[5];

};


/**************************************************************/

class EffectsManager {
private:
	static EffectsManager* _instance;

public:
	static EffectsManager* Instance() { 
		if (!_instance) { 
			_instance = new EffectsManager(); 
		} 
		return _instance; 
	}
	EffectsManager();
	~EffectsManager();

	//StdMeshEffect* m_stdMeshEffect;
	ShadowMapEffect* m_shadowMapEffect;
	DeferredGeometryPassEffect* m_deferredGeometryPassEffect; 
	DeferredGeometrySkinnedPassEffect* m_deferredGeometrySkinnedPassEffect;
	DeferredGeometryTerrainPassEffect* m_deferredGeometryTerrainPassEffect;
	DeferredGeometryTessPassEffect* m_deferredGeometryTessPassEffect;
	DeferredShadingPassEffect* m_deferredShadingPassEffect;
	DeferredShadingCS* m_deferredShadingCSEffect;
	GodRayEffect* m_godrayEffect;
	SkyboxEffect* m_skyboxEffect;
	BlurEffect* m_blurEffect;
	VBlurEffect* m_vblurEffect;
	HBlurEffect* m_hblurEffect;
	DebugLineEffect* m_debugLineEffect;
};

#endif