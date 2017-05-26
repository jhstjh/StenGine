#pragma once

#include <vector>
#include "Graphics/Abstraction/GPUBuffer.h"
#include "Scene/Transform.h"

namespace StenGine
{

class Mesh;

class TerrainGrass
{
public:
	TerrainGrass(float x, float y, float z, float rx = 0, float ry = 0, float rz = 0);
	void GatherDrawCall();

private:
	void PrepareGPUBuffer();

	Transform mTransform;
	Mesh* mMesh{ nullptr };

	GPUBuffer mIndexBufferGPU;
	GPUBuffer mVertexBufferGPU;
	GPUBuffer mInstanceBuffer;

	std::vector<Vertex::InstanceVertex> mInstances;
};

}