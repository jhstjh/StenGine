#ifndef __EFFECTS_MANAGER__
#define __EFFECTS_MANAGER__
#include "D3DIncludes.h"
#include "MeshRenderer.h"
#include "Material.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

class Mesh;

class Effect {
protected:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11HullShader* m_hullShader;
	ID3D11DomainShader* m_domainShader;
	
	ID3DBlob *m_vsBlob;
	ID3DBlob *m_psBlob;
	ID3DBlob *m_gsBlob;
	ID3DBlob *m_hsBlob;
	ID3DBlob *m_dsBlob;

	ID3DX11Effect* m_fx;
	ID3D11InputLayout* m_inputLayout;
	ID3DX11EffectTechnique* m_activeTech;
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_vertexDesc;
public:
	Effect(const std::wstring& filename);
	Effect(const std::wstring& vsPath,
		   const std::wstring& psPath,
		   const std::wstring& gsPath,
		   const std::wstring& hsPath,
		   const std::wstring& dsPath);
	virtual ~Effect();
	virtual void SetShader();
	virtual void CreateConstantBuffer() = 0;
	virtual void BindConstantBuffer() = 0;
	virtual void BindShaderResource() = 0;
	virtual void UnBindConstantBuffer();
	virtual void UnBindShaderResource();
	virtual void UnSetShader();
	ID3D11InputLayout* GetInputLayout() { return m_inputLayout; }
	ID3DX11EffectTechnique* GetActiveTech() { return m_activeTech; }
	std::vector<Mesh*> m_associatedMeshes;
};


//--------------------------------------------------------------------//


class StdMeshEffect : public Effect {
public:
	StdMeshEffect(const std::wstring& filename);
	~StdMeshEffect();

	ID3DX11EffectTechnique* StdMeshTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* TheShadowMap;
};


//--------------------------------------------------------------------//


class DeferredShaderEffect : public Effect {
private:
	ID3D11Buffer* m_perFrameCB;
	ID3D11Buffer* m_perObjectCB;

public:
	DeferredShaderEffect(const std::wstring& filename);
	~DeferredShaderEffect();

	ID3DX11EffectTechnique* DeferredShaderTech;
	ID3DX11EffectTechnique* DeferredShaderTessTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* WorldView;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectMatrixVariable* WorldViewInvTranspose;
	//ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* DiffX_NormY_ShadZ;
	ID3DX11EffectShaderResourceVariable* CubeMap;
	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
	ID3DX11EffectShaderResourceVariable* TheShadowMap;
	ID3DX11EffectShaderResourceVariable* BumpMap;

	virtual void CreateConstantBuffer();
	virtual void BindConstantBuffer();
	virtual void BindShaderResource();
// 	virtual void UnBindConstantBuffer();
// 	virtual void UnBindShaderResource();

	struct PEROBJ_CONSTANT_BUFFER
	{
		XMMATRIX WorldViewProj;
		XMMATRIX WorldViewInvTranspose;
		XMMATRIX WorldInvTranspose;
		XMMATRIX WorldView;
		XMMATRIX World;
		XMMATRIX ViewProj;
		Material Mat;
		XMMATRIX ShadowTransform;
		XMFLOAT4 DiffX_NormY_ShadZ;
	} m_perObjConstantBuffer;

	struct PERFRAME_CONSTANT_BUFFER
	{
		XMFLOAT4 EyePosW;
	} m_perFrameConstantBuffer;

	ID3D11ShaderResourceView **m_shaderResources;
// 	ID3DX11EffectShaderResourceVariable* CubeMap;
// 	ID3DX11EffectShaderResourceVariable* DiffuseMap;
// 	ID3DX11EffectShaderResourceVariable* NormalMap;
// 	ID3DX11EffectShaderResourceVariable* TheShadowMap;
// 	ID3DX11EffectShaderResourceVariable* BumpMap;
};


//--------------------------------------------------------------------//


class ShadowMapEffect : public Effect {
public:
	ShadowMapEffect(const std::wstring& filename);
	~ShadowMapEffect();

	ID3DX11EffectTechnique* ShadowMapTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
};


//--------------------------------------------------------------------//


class ScreenQuadEffect : public Effect {
public:
	ScreenQuadEffect(const std::wstring& filename);
	~ScreenQuadEffect();

	ID3DX11EffectTechnique* FullScreenQuadTech;
	ID3DX11EffectTechnique* DeferredLightingTech;
	ID3DX11EffectTechnique* HBlurTech;
	ID3DX11EffectTechnique* VBlurTech;
	ID3DX11EffectShaderResourceVariable* ScreenMap;
	ID3DX11EffectShaderResourceVariable* SSAOMap;
	ID3DX11EffectShaderResourceVariable* DiffuseGB;
	ID3DX11EffectShaderResourceVariable* PositionGB;
	ID3DX11EffectShaderResourceVariable* NormalGB;
	ID3DX11EffectShaderResourceVariable* SpecularGB;
	ID3DX11EffectShaderResourceVariable* DepthGB;
	ID3DX11EffectVariable* DirLight;
	//ID3DX11EffectVariable* Mat;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectMatrixVariable* ProjInv;
	ID3DX11EffectMatrixVariable* Proj;
};


//--------------------------------------------------------------------//


class GodRayEffect : public Effect {
public:
	GodRayEffect(const std::wstring& filename);
	~GodRayEffect();

	ID3DX11EffectTechnique* GodRayTech;
	ID3DX11EffectShaderResourceVariable* OcclusionMap;
	ID3DX11EffectVariable* LightPosH;
};


//--------------------------------------------------------------------//


class SkyboxEffect : public Effect {
public:
	SkyboxEffect(const std::wstring& filename);
	~SkyboxEffect();

	ID3DX11EffectTechnique* SkyboxTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectShaderResourceVariable* CubeMap;
};

//--------------------------------------------------------------------//


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

	StdMeshEffect* m_stdMeshEffect;
	ShadowMapEffect* m_shadowMapEffect;
	DeferredShaderEffect* m_deferredShaderEffect; 
	ScreenQuadEffect* m_screenQuadEffect;
	GodRayEffect* m_godrayEffect;
	SkyboxEffect* m_skyboxEffect;
};

#endif