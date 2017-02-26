#ifndef __MESH_RENDERER__
#define __MESH_RENDERER__

#include <vector>

#include "Graphics/Abstraction/GPUBuffer.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Effect/Material.h"
#include "Math/MathDefs.h"
#include "Mesh/SubMesh.h"
#include "System/API/PlatformAPIDefs.h"
#include "Scene/Component.h"
#include "Scene/Drawable.h"


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
	std::vector<Vec3Packed> m_positionBufferCPU;
	std::vector<Vec3Packed> m_normalBufferCPU;
	std::vector<Vec2Packed> m_texUVBufferCPU;
	std::vector<Vec4Packed> m_colorBufferCPU;
	std::vector<Vec3Packed> m_tangentBufferCPU;

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
