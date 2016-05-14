#pragma once

#include "Scene/Component.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/Skeleton.h"
#include "Graphics/Animation/Animation.h"

using namespace DirectX;

namespace StenGine
{

struct Joint {
	XMMATRIX m_inverseBindPosMat;
	int32_t m_index;
	int32_t m_parentIdx;
	std::string m_name;
};

class SkinnedMesh : public Mesh {
public:
	SkinnedMesh();
	virtual ~SkinnedMesh();

	std::vector<std::vector<float> > m_jointWeightsBufferCPU;
	std::vector<std::vector<float> > m_jointIndicesBufferCPU;

	std::vector<XMMATRIX> m_jointPreRotationBufferCPU;
	std::vector<XMMATRIX> m_jointRotationBufferCPU;
	std::vector<XMMATRIX> m_jointTranformBufferCPU;
	std::vector<XMMATRIX> m_jointOffsetTransformCPU;
	std::vector<XMMATRIX> m_matrixPalette;
	std::vector<Joint> m_joints;

	virtual void GatherDrawCall() override;
	virtual void GatherShadowDrawCall() override;

	virtual void DrawMenu() override {}

	void SetAnimation(Animation* animation) { m_animation = animation; }

private:

	void PrepareMatrixPalette();

	virtual void PrepareGPUBuffer() override;
	virtual void PrepareShadowMapBuffer() override;

	Animation* m_animation;
};

}