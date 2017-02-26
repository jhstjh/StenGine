#pragma once

#include "Graphics/Animation/Animation.h"
#include "Mesh/MeshRenderer.h"
#include "Math/MathDefs.h"
#include "Mesh/Skeleton.h"
#include "Scene/Component.h"

namespace StenGine
{

struct Joint {
	Mat4 m_inverseBindPosMat;
	int32_t m_index;
	int32_t m_parentIdx;
	std::string m_name;
};

class SkinnedMesh : public Mesh {
public:
	SkinnedMesh();
	virtual ~SkinnedMesh();

	std::vector<std::vector<float> > m_jointWeightsBufferCPU;
	std::vector<std::vector<uint32_t> > m_jointIndicesBufferCPU;

	std::vector<Mat4> m_jointPreRotationBufferCPU;
	std::vector<Mat4> m_jointOffsetTransformCPU;
	std::vector<Mat4> m_toRootTransform;
	std::vector<Mat4> m_matrixPalette;
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