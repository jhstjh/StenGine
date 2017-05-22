#include <vector>
#include "Mesh/Mesh.h"
#include "Resource/ResourceManager.h"

namespace StenGine
{

Mesh::Mesh(int32_t type/* = 0*/)
{
	if (type == 0)
		CreateBoxPrimitive();
	else if (type == 1)
		CreatePlanePrimitive();
}

void Mesh::CreateBoxPrimitive()
{
	m_positionBufferCPU.resize(24);
	m_positionBufferCPU =
	{
		Vec3Packed({ -1.0f, -1.0f, -1.0f }),
		Vec3Packed({ -1.0f, +1.0f, -1.0f }),
		Vec3Packed({ +1.0f, +1.0f, -1.0f }),
		Vec3Packed({ +1.0f, -1.0f, -1.0f }),

		Vec3Packed({ -1.0f, -1.0f, +1.0f }),
		Vec3Packed({ +1.0f, -1.0f, +1.0f }),
		Vec3Packed({ +1.0f, +1.0f, +1.0f }),
		Vec3Packed({ -1.0f, +1.0f, +1.0f }),

		Vec3Packed({ -1.0f, +1.0f, -1.0f }),
		Vec3Packed({ -1.0f, +1.0f, +1.0f }),
		Vec3Packed({ +1.0f, +1.0f, +1.0f }),
		Vec3Packed({ +1.0f, +1.0f, -1.0f }),

		Vec3Packed({ -1.0f, -1.0f, -1.0f }),
		Vec3Packed({ +1.0f, -1.0f, -1.0f }),
		Vec3Packed({ +1.0f, -1.0f, +1.0f }),
		Vec3Packed({ -1.0f, -1.0f, +1.0f }),

		Vec3Packed({ -1.0f, -1.0f, +1.0f }),
		Vec3Packed({ -1.0f, +1.0f, +1.0f }),
		Vec3Packed({ -1.0f, +1.0f, -1.0f }),
		Vec3Packed({ -1.0f, -1.0f, -1.0f }),

		Vec3Packed({ +1.0f, -1.0f, -1.0f }),
		Vec3Packed({ +1.0f, +1.0f, -1.0f }),
		Vec3Packed({ +1.0f, +1.0f, +1.0f }),
		Vec3Packed({ +1.0f, -1.0f, +1.0f }),
	};

	m_normalBufferCPU.resize(24);
	m_normalBufferCPU =
	{
		Vec3Packed({ 0.0f, 0.0f, -1.0f }),
		Vec3Packed({ 0.0f, 0.0f, -1.0f }),
		Vec3Packed({ 0.0f, 0.0f, -1.0f }),
		Vec3Packed({ 0.0f, 0.0f, -1.0f }),

		Vec3Packed({ 0.0f, 0.0f, 1.0f }),
		Vec3Packed({ 0.0f, 0.0f, 1.0f }),
		Vec3Packed({ 0.0f, 0.0f, 1.0f }),
		Vec3Packed({ 0.0f, 0.0f, 1.0f }),

		Vec3Packed({ 0.0f, 1.0f, 0.0f }),
		Vec3Packed({ 0.0f, 1.0f, 0.0f }),
		Vec3Packed({ 0.0f, 1.0f, 0.0f }),
		Vec3Packed({ 0.0f, 1.0f, 0.0f }),

		Vec3Packed({ 0.0f, -1.0f, 0.0f }),
		Vec3Packed({ 0.0f, -1.0f, 0.0f }),
		Vec3Packed({ 0.0f, -1.0f, 0.0f }),
		Vec3Packed({ 0.0f, -1.0f, 0.0f }),

		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),

		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
	};

	m_tangentBufferCPU.resize(24);
	m_tangentBufferCPU =
	{
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),

		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),

		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),

		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),
		Vec3Packed({ -1.0f, 0.0f, 0.0f }),

		Vec3Packed({ 0.0f, 0.0f, -1.0f }),
		Vec3Packed({ 0.0f, 0.0f, -1.0f }),
		Vec3Packed({ 0.0f, 0.0f, -1.0f }),
		Vec3Packed({ 0.0f, 0.0f, -1.0f }),

		Vec3Packed({ 0.0f, 0.0f, 1.0f }),
		Vec3Packed({ 0.0f, 0.0f, 1.0f }),
		Vec3Packed({ 0.0f, 0.0f, 1.0f }),
		Vec3Packed({ 0.0f, 0.0f, 1.0f }),
	};

	m_texUVBufferCPU.resize(24);
	m_texUVBufferCPU =
	{
		Vec2Packed({ 0.0f, 1.0f }),
		Vec2Packed({ 0.0f, 0.0f }),
		Vec2Packed({ 1.0f, 0.0f }),
		Vec2Packed({ 1.0f, 1.0f }),

		Vec2Packed({ 1.0f, 1.0f }),
		Vec2Packed({ 0.0f, 1.0f }),
		Vec2Packed({ 0.0f, 0.0f }),
		Vec2Packed({ 1.0f, 0.0f }),

		Vec2Packed({ 0.0f, 1.0f }),
		Vec2Packed({ 0.0f, 0.0f }),
		Vec2Packed({ 1.0f, 0.0f }),
		Vec2Packed({ 1.0f, 1.0f }),

		Vec2Packed({ 1.0f, 1.0f }),
		Vec2Packed({ 0.0f, 1.0f }),
		Vec2Packed({ 0.0f, 0.0f }),
		Vec2Packed({ 1.0f, 0.0f }),

		Vec2Packed({ 0.0f, 1.0f }),
		Vec2Packed({ 0.0f, 0.0f }),
		Vec2Packed({ 1.0f, 0.0f }),
		Vec2Packed({ 1.0f, 1.0f }),

		Vec2Packed({ 0.0f, 1.0f }),
		Vec2Packed({ 0.0f, 0.0f }),
		Vec2Packed({ 1.0f, 0.0f }),
		Vec2Packed({ 1.0f, 1.0f }),
	};

	m_indexBufferCPU.resize(36);
	m_indexBufferCPU =
	{

		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	m_subMeshes.resize(1);
	m_materials.resize(1);

	m_materials[0].m_attributes.ambient = Vec4(0.2f, 0.2f, 0.2f, 1.f);
	m_materials[0].m_attributes.diffuse = Vec4(1.0f, 0.5f, 0.3f, 1.f);
	m_materials[0].m_attributes.specular = Vec4(0.6f, 0.6f, 0.6f, 10.0f);
	m_subMeshes[0].m_indexBufferCPU = m_indexBufferCPU;
	m_subMeshes[0].m_matIndex = 0;

	m_materials[0].m_diffuseMapTex = ResourceManager::Instance()->GetSharedResource<Texture>(L"../StenGine/Model/WoodCrate02.dds");
	m_materials[0].m_normalMapTex = ResourceManager::Instance()->GetSharedResource<Texture>(L"../StenGine/Model/WoodCrate02_normal.dds");
}

void Mesh::CreatePlanePrimitive() {
	m_positionBufferCPU.resize(4);
	m_positionBufferCPU = {
		Vec3Packed({ -4.0f, -1.0f, -4.0f }),
		Vec3Packed({ +4.0f, -1.0f, -4.0f }),
		Vec3Packed({ +4.0f, -1.0f, +4.0f }),
		Vec3Packed({ -4.0f, -1.0f, +4.0f }),
	};

	m_normalBufferCPU.resize(4);
	m_normalBufferCPU = {
		Vec3Packed({ 0.0f, 1.0f, 0.0f }),
		Vec3Packed({ 0.0f, 1.0f, 0.0f }),
		Vec3Packed({ 0.0f, 1.0f, 0.0f }),
		Vec3Packed({ 0.0f, 1.0f, 0.0f }),
	};

	m_tangentBufferCPU.resize(4);
	m_tangentBufferCPU = {
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
		Vec3Packed({ 1.0f, 0.0f, 0.0f }),
	};

	m_texUVBufferCPU.resize(24);
	m_texUVBufferCPU = {
		Vec2Packed({ 1.0f, 1.0f }),
		Vec2Packed({ 0.0f, 1.0f }),
		Vec2Packed({ 0.0f, 0.0f }),
		Vec2Packed({ 1.0f, 0.0f }),
	};

	m_indexBufferCPU.resize(6);
	m_indexBufferCPU = {

		1, 0, 2,
		2, 0, 3,
	};

	m_materials.resize(1);
	m_materials[0].m_attributes.ambient = Vec4(0.2f, 0.2f, 0.2f, 1.f);
	m_materials[0].m_attributes.diffuse = Vec4(1.0f, 0.5f, 0.3f, 1.f);
	m_materials[0].m_attributes.specular = Vec4(0.6f, 0.6f, 0.6f, 16.0f);
}


}