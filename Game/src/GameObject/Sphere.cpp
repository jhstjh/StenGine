#include "GameObject/sphere.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Sphere::Sphere()
{
	Mesh* sphereMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/earth.fbx");
	auto sphereMeshRenderer = std::make_unique<MeshRenderer>();
	sphereMeshRenderer->SetMesh(sphereMesh);
	AddComponent(std::move(sphereMeshRenderer));
}

void Sphere::Update()
{
	m_transform->RotateAroundY(-Timer::GetDeltaTime() * 3.14159f);
}

}