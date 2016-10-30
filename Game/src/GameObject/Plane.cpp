#include "GameObject/Plane.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Plane::Plane()
{
	Mesh* planeMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plane.fbx");
	AddComponent(planeMesh);
}

}