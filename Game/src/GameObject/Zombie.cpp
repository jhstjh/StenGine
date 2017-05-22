#include "GameObject/zombie.h"
#include "Mesh/SkinnedMeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Zombie::Zombie()
{
	SkinnedMesh* zombieMesh = ResourceManager::Instance()->GetResource<SkinnedMesh>(L"Model/vampire-animated.fbx");
	auto zombieMeshRenderer = std::make_unique<SkinnedMeshRenderer>();
	zombieMeshRenderer->SetMesh(zombieMesh);
	
	Animation* animation = ResourceManager::Instance()->GetResource<Animation>(L"Model/vampire-animated.fbx");
	zombieMeshRenderer.get()->SetAnimation(animation);

	AddComponent(std::move(zombieMeshRenderer));
}

}