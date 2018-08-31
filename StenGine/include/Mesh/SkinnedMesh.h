#pragma once
#include "Mesh/Mesh.h"
#include "Mesh/SubMesh.h"

namespace StenGine
{

class FbxReaderSG;
class SkinnedMeshRenderer;

struct Joint {
	Mat4 m_inverseBindPosMat;
	int32_t m_index;
	int32_t m_parentIdx;
	std::string m_name;
};

class SkinnedMesh : public Mesh
{
public:
	SkinnedMesh() : Mesh(2) {}; // TODO errrrrrr

	friend FbxReaderSG;
	friend SkinnedMeshRenderer;

private:
	std::vector<std::vector<float> > m_jointWeightsBufferCPU;
	std::vector<std::vector<uint32_t> > m_jointIndicesBufferCPU;

	std::vector<Mat4> m_jointPreRotationBufferCPU;
	std::vector<Mat4> m_jointOffsetTransformCPU;
	std::vector<Joint> m_joints;
};


}