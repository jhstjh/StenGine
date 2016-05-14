#ifndef __MESH_RENDERER__
#define __MESH_RENDERER__

#include <vector>

#include "System/API/PlatformAPIDefs.h"
#include "Scene/Drawable.h"

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
#include "Graphics/Abstraction/GPUBuffer.h"

namespace StenGine
{

class Effect;

class Mesh : public Component, public Drawable {
protected:
	GPUBuffer* m_indexBufferGPU;
	GPUBuffer* m_vertexBufferGPU;
	GPUBuffer* m_shadowMapVertexBufferGPU;

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

	std::vector<SubMesh> m_subMeshes;
	std::vector<Material> m_materials;
	XMFLOAT4X4 m_worldTransform;
	bool m_castShadow;
	bool m_receiveShadow;
	Mesh(int type);
	virtual ~Mesh();
	virtual void GatherDrawCall() override;
	virtual void GatherShadowDrawCall() override;
	void Prepare();

	virtual void DrawMenu() override;
};

}
#endif // !__MESH_RENDERER__
