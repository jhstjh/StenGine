#pragma once

#include "Scene/Component.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/Skeleton.h"

namespace StenGine
{

class SkinnedMesh : public Mesh {
public:
	SkinnedMesh();
	~SkinnedMesh();

private:

	virtual void PrepareGPUBuffer() override;
	virtual void PrepareShadowMapBuffer() override;

	Skeleton* m_skeleton;

	std::vector<XMFLOAT4> m_jointWeightsBufferCPU;
	std::vector<XMUINT4> m_jointIndicesBufferCPU;
};

}