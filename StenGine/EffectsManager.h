#ifndef __EFFECTS_MANAGER__
#define __EFFECTS_MANAGER__
#include "D3DIncludes.h"
#include "MeshRenderer.h"

class Mesh;

class Effect {
protected:
	ID3DX11Effect* m_fx;
	ID3D11InputLayout* m_inputLayout;
	ID3DX11EffectTechnique* m_activeTech;
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_vertexDesc;
public:
	Effect(const std::wstring& filename);
	virtual ~Effect();
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
public:
	DeferredShaderEffect(const std::wstring& filename);
	~DeferredShaderEffect();

	ID3DX11EffectTechnique* DeferredShaderTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* WorldView;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectMatrixVariable* WorldViewInvTranspose;
	//ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* TheShadowMap;
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
	ID3DX11EffectShaderResourceVariable* ScreenMap;
	ID3DX11EffectShaderResourceVariable* DiffuseGB;
	ID3DX11EffectShaderResourceVariable* PositionGB;
	ID3DX11EffectShaderResourceVariable* NormalGB;
	ID3DX11EffectShaderResourceVariable* SpecularGB;
	ID3DX11EffectShaderResourceVariable* DepthGB;
	ID3DX11EffectVariable* DirLight;
	//ID3DX11EffectVariable* Mat;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectMatrixVariable* ProjInv;
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
};

#endif