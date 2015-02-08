#pragma once

#include "Component.h"
#include "MeshRenderer.h"
#include "Skeleton.h"

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