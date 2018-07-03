#include "GameObject/zombie.h"
#include "Graphics/Animation/Animator.h"
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
	auto zombieAnimator = std::make_unique<Animator>();
	zombieAnimator->SetAnimation(animation);
	zombieAnimator->CreateClip(2, 120, "Idle");
	zombieAnimator->SetCurrentClip("Idle");

	AddComponent(std::move(zombieMeshRenderer));
	AddComponent(std::move(zombieAnimator));
}

}