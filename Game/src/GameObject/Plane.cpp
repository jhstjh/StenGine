#include "GameObject/Plane.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Plane::Plane()
{
	Mesh* planeMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plane.fbx");
	auto planeMeshRenderer = std::make_unique<MeshRenderer>();
	planeMeshRenderer->SetMesh(planeMesh);
	AddComponent(std::move(planeMeshRenderer));
}

}