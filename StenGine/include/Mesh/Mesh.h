#pragma once
#include <memory>
#include "Math/MathDefs.h"
#include "Mesh/SubMesh.h"

namespace StenGine
{

class FbxReaderSG;
class MeshRenderer;
class TerrainGrass;

class Mesh
{
public:
	Mesh::Mesh(int32_t type = 0);
	virtual ~Mesh() = default;

	friend FbxReaderSG;
	friend MeshRenderer;
	friend TerrainGrass;

	std::vector<SubMesh> m_subMeshes;
	std::vector<Material> m_materials;

protected:
	void CreateBoxPrimitive();
	void CreatePlanePrimitive();

	std::vector<uint32_t> m_indexBufferCPU;
	std::vector<Vec3Packed> m_positionBufferCPU;
	std::vector<Vec3Packed> m_normalBufferCPU;
	std::vector<Vec2Packed> m_texUVBufferCPU;
	std::vector<Vec4Packed> m_colorBufferCPU;
	std::vector<Vec3Packed> m_tangentBufferCPU;
};


}