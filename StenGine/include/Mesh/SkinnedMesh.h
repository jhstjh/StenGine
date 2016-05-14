#pragma once

#include "Scene/Component.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/Skeleton.h"

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
	~SkinnedMesh();

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

private:

	void PrepareMatrixPalette();

	virtual void PrepareGPUBuffer() override;
	virtual void PrepareShadowMapBuffer() override;

	Skeleton* m_skeleton;

	GLuint m_ssbo;

};

}