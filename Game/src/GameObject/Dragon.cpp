#include "GameObject/Dragon.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Dragon::Dragon()
{
	Mesh* dragonMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/dragon.fbx");
	auto dragonMeshRenderer = std::make_unique<MeshRenderer>();
	dragonMeshRenderer->SetMesh(dragonMesh);
	AddComponent(std::move(dragonMeshRenderer));
}

}