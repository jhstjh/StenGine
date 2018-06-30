#pragma once

#include <vector>
#include "Graphics/Abstraction/GPUBuffer.h"
#include "Scene/Transform.h"

namespace StenGine
{

class Mesh;
class Terrain;

class TerrainGrass
{
public:
	TerrainGrass(Terrain* terrain, float width, float depth);
	void GatherDrawCall();

private:
	void PrepareGPUBuffer();

	Terrain* mParent;
	Mesh* mMesh{ nullptr };

	GPUBuffer mIndexBufferGPU;
	GPUBuffer mVertexBufferGPU;
	GPUBuffer mInstanceBuffer;

	std::vector<Vertex::InstanceVertex> mInstances;
};

}