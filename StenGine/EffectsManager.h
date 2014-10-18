#ifndef __EFFECTS_MANAGER__
#define __EFFECTS_MANAGER__
#include "D3DIncludes.h"
#include "MeshRenderer.h"

class MeshRenderer;

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
	std::vector<MeshRenderer*> m_associatedMeshes;
};


//--------------------------------------------------------------------//


class StdMeshEffect : public Effect {
public:
	StdMeshEffect(const std::wstring& filename);
	~StdMeshEffect();

	ID3DX11EffectTechnique* StdMeshTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
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
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	//ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	//ID3DX11EffectShaderResourceVariable* TheShadowMap;
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
	ID3DX11EffectShaderResourceVariable* ScreenMap;
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
};

#endif