#include "GameObject/sphere.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Sphere::Sphere()
{
	Mesh* sphereMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/earth.fbx");
	AddComponent(sphereMesh);
}

void Sphere::Update()
{
	mTransform->RotateAroundY(-Timer::GetDeltaTime() * 3.14159f);
}

}