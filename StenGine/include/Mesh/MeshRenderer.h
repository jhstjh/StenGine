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
class Mesh;

class MeshRenderer : public Component, public Drawable {
public:
	GPUBuffer m_indexBufferGPU;
	GPUBuffer m_vertexBufferGPU;
	GPUBuffer m_shadowMapVertexBufferGPU;

	Effect* m_associatedEffect{ nullptr };
	Effect* m_associatedDeferredEffect{ nullptr };
	Mesh* mMesh{ nullptr };

	virtual void PrepareGPUBuffer();
	virtual void PrepareShadowMapBuffer();

	bool m_castShadow{ true };
	bool m_receiveShadow{ true };
	virtual ~MeshRenderer();
	virtual void SetMesh(Mesh* mesh);
	virtual void GatherDrawCall() override;
	virtual void GatherShadowDrawCall() override;
	void Prepare();

	virtual void DrawMenu() override;
};

}
#endif // !__MESH_RENDERER__
