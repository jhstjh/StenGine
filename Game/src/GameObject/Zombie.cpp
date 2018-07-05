#include "GameObject/zombie.h"
#include "Graphics/Animation/Animator.h"
#include "Input/InputManager.h"
#include "Mesh/SkinnedMeshRenderer.h"
#include "Mesh/Terrain/Terrain.h"
#include "Resource/ResourceManager.h"
#include "Scene/GameObjectManager.h"


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
	zombieAnimator->CreateClip(603, 631, "Walk");

	zombieAnimator->SetPositionDrivenNodeName("Hips");
	zombieAnimator->SetCurrentClip("Idle");
	mState = State::IDLE;

	AddComponent(std::move(zombieMeshRenderer));
	AddComponent(std::move(zombieAnimator));
}

void Zombie::Update()
{
	switch (mState)
	{
	case State::IDLE:
		processIDLE();
		break;
	case State::WALK:
		processWALK();
		break;
	default:
		break;
	}
}

void Zombie::processIDLE()
{
	if (InputManager::Instance()->GetKeyHold('I'))
	{
		GetFirstComponentByType<Animator>()->SetCurrentClip("Walk", 0.5);
		mState = State::WALK;
		return;
	}
}

void Zombie::processWALK()
{
	auto dt = Timer::GetDeltaTime();
	if (!InputManager::Instance()->GetKeyHold('I'))
	{
		GetFirstComponentByType<Animator>()->SetCurrentClip("Idle", 0.5);
		mState = State::IDLE;
		return;
	}

	auto transform = GetTransform();
	if (InputManager::Instance()->GetKeyHold('J'))
	{
		transform->Rotate(-120.f / 180.f * PI * dt, {0.f, 1.f, 0.f}, false);
	}

	if (InputManager::Instance()->GetKeyHold('L'))
	{
		transform->Rotate(120.f / 180.f * PI * dt, { 0.f, 1.f, 0.f }, false);
	}

	GameObject* terrain = GameObjectManager::Instance()->FindGameObjectByName("Terrain"); // todo cache it
	if (terrain)
	{
		auto terrainComponent = terrain->GetFirstComponentByType<Terrain>();
		Vec3 terrainPos = terrain->GetTransform()->GetPosition();

		Vec3 pos = transform->GetPosition();

		float height = terrainComponent->GetHeight(pos.x() - terrainPos.x(), pos.z() - terrainPos.z());
		transform->SetPosY(height);
	}
}

}