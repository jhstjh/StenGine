#include "GameObject/zombie.h"
#include "Mesh/SkinnedMesh.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Zombie::Zombie()
{
	SkinnedMesh* zombieMesh = ResourceManager::Instance()->GetResource<SkinnedMesh>(L"Model/vampire-animated.fbx");
	AddComponent(zombieMesh);

	Animation* animation = ResourceManager::Instance()->GetResource<Animation>(L"Model/vampire-animated.fbx");
	zombieMesh->SetAnimation(animation);
}

}