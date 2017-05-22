#include "GameObject/Plant.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Plant::Plant()
{
	Mesh* plantMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plants.fbx");
	auto plantMeshRenderer = std::make_unique<MeshRenderer>();
	plantMeshRenderer->SetMesh(plantMesh);
	AddComponent(std::move(plantMeshRenderer));
}

}