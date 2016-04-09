#ifndef __MESH_RENDERER__
#define __MESH_RENDERER__

#include "System/API/PlatformAPIDefs.h"
#include <vector>

#if (PLATFORM_WIN32) || (SG_TOOL)
#include "Graphics/D3DIncludes.h"
#include "Graphics/Abstraction/RendererBase.h"
#elif  PLATFORM_ANDROID
#include "AndroidType.h"
#endif
#include "Graphics/Effect/EffectsManager.h"

#include "Scene/Component.h"
#include "Graphics/Effect/Material.h"
#include "Mesh/SubMesh.h"

class Effect;

class Mesh: public Component {
protected:
#if GRAPHICS_D3D11
	ID3D11Buffer* m_indexBufferGPU;
	ID3D11Buffer* m_stdMeshVertexBufferGPU;
	ID3D11Buffer* m_shadowMapVertexBufferGPU;
#else
	GLuint m_indexBufferGPU;
	GLuint m_stdMeshVertexBufferGPU;
	GLuint m_shadowMapVertexBufferGPU;

	GLuint m_positionBufferGPU;
	GLuint m_normalBufferGPU;
	GLuint m_texUVBufferGPU;
	GLuint m_tangentBufferGPU;

	GLuint m_vertexArrayObject;
	GLuint m_shadowVertexArrayObject;
#endif
	Effect* m_associatedEffect;
	Effect* m_associatedDeferredEffect;

	void CreateBoxPrimitive();
	void CreatePlanePrimitive();
	virtual void PrepareGPUBuffer();
	virtual void PrepareShadowMapBuffer();
	void PrepareSRV(int type);

public:
	std::vector<UINT> m_indexBufferCPU;
	std::vector<XMFLOAT3> m_positionBufferCPU;
	std::vector<XMFLOAT3> m_normalBufferCPU;
	std::vector<XMFLOAT2> m_texUVBufferCPU;
	std::vector<XMFLOAT4> m_colorBufferCPU;
	std::vector<XMFLOAT3> m_tangentBufferCPU;
#if GRAPHICS_D3D11
	ID3D11ShaderResourceView* m_diffuseMapSRV;
	ID3D11ShaderResourceView* m_normalMapSRV;
	ID3D11ShaderResourceView* m_bumpMapSRV;
#else
	GLuint m_diffuseMap;
	GLuint m_normalMap;
#endif

	Material m_material;
	std::vector<SubMesh> m_subMeshes;
	XMFLOAT4X4 m_worldTransform;
	bool m_castShadow;
	bool m_receiveShadow;
	Mesh(int type);
	~Mesh();
	virtual void GatherDrawCall();
	virtual void GatherShadowDrawCall();
	void Prepare();
};

#endif // !__MESH_RENDERER__
