#include "GameObject/House.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

House::House()
{
	Mesh* houseMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/house.fbx");
	auto houseMeshRenderer = std::make_unique<MeshRenderer>();
	houseMeshRenderer->SetMesh(houseMesh);
	AddComponent(std::move(houseMeshRenderer));
}

}