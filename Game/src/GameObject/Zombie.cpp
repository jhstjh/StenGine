#include "GameObject/zombie.h"
#include "Graphics/Animation/Animator.h"
#include "Input/InputManager.h"
#include "Mesh/SkinnedMeshRenderer.h"
#include "Mesh/Terrain/Terrain.h"
#include "Resource/ResourceManager.h"
#include "Scene/GameObjectManager.h"
#include "Math/MathHelper.h"

static const float WALK_THRESHOLD = 0.1f;

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
	zombieAnimator->CreateClip(603, 632, "Walk");

	zombieAnimator->SetPositionDrivenNodeName("Hips");
	zombieAnimator->SetCurrentClip("Idle");
	mState = State::IDLE;

	AddComponent(std::move(zombieMeshRenderer));
	AddComponent(std::move(zombieAnimator));
}

void Zombie::Update()
{
	auto dt = Timer::GetDeltaTime();
	Vec3 dir{ 0, 0, 0 };

	if (InputManager::Instance()->GetKeyHold('W'))
	{
		dir += { 0, 0, 1 };
	}
	if (InputManager::Instance()->GetKeyHold('S'))
	{
		dir -= { 0, 0, 1 };
	}
	if (InputManager::Instance()->GetKeyHold('A'))
	{
		dir -= { 1, 0, 0 };
	}
	if (InputManager::Instance()->GetKeyHold('D'))
	{
		dir += { 1, 0, 0 };
	}

	float LX = InputManager::Instance()->GetAxisValue(0, GamepadXinput::GamepadAxis::PadLX);
	float LY = InputManager::Instance()->GetAxisValue(0, GamepadXinput::GamepadAxis::PadLY);

	if (fabs(LX) < WALK_THRESHOLD) LX = 0.f;
	if (fabs(LY) < WALK_THRESHOLD) LY = 0.f;

	dir += { LX, 0.f, LY};

	if (dir != Vec3{0, 0, 0})
	{
		dir.Normalize();

		auto camera = GameObjectManager::Instance()->FindGameObjectByName("ThirdPersonCamera");
		assert(camera);

		auto forward = GetTransform()->GetPosition() - camera->GetTransform()->GetPosition();
		forward.y() = 0.f;
		forward.Normalize();

		Vec3 right = Vec3::CrossProduct({ 0.f, 1.f, 0.f }, forward).Normalized();
		Vec3 realUp = Vec3::CrossProduct(forward, right).Normalized();

		Mat3 rot = Mat3(right, realUp, forward);

		Vec3 realDir = rot * dir;

		auto currentForward = MatrixHelper::GetForward(GetTransform()->GetWorldTransform());
		if (realDir == -currentForward)
		{
			realDir.x() += 0.001f;
			realDir.Normalize();
		}

		auto cross = Vec3::CrossProduct(realDir, currentForward);
		auto dot = Vec3::DotProduct(realDir, currentForward);
		float angle = atan2f(cross.Length(), dot);
		float turnDir = cross.y() < 0 ? 1.f : -1.f;

		auto rotMat = Mat4::RotationY(std::min(toRadian(360.f * dt), angle) * turnDir);
		auto target = GetTransform()->GetPosition() + rotMat * currentForward;

		GetTransform()->LookAt(target, { 0.f, 1.f, 0.f });

		mWalkSpeed = sqrt(LX * LX + LY * LY);
		if (mWalkSpeed == 0.f)
		{
			mWalkSpeed = 1.f;
		}
	}

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
	if (canWalk())
	{
		GetFirstComponentByType<Animator>()->SetCurrentClip("Walk", 0.5);
		mState = State::WALK;
		return;
	}
}

void Zombie::processWALK()
{
	if (!canWalk())
	{
		GetFirstComponentByType<Animator>()->SetCurrentClip("Idle", 0.5);
		mState = State::IDLE;
		return;
	}

	auto transform = GetTransform();

	GameObject* terrain = GameObjectManager::Instance()->FindGameObjectByName("Terrain"); // todo cache it
	if (terrain)
	{
		auto terrainComponent = terrain->GetFirstComponentByType<Terrain>();
		Vec3 terrainPos = terrain->GetTransform()->GetPosition();

		Vec3 pos = transform->GetPosition();

		float height = terrainComponent->GetHeight(pos.x() - terrainPos.x(), pos.z() - terrainPos.z());
		transform->SetPosY(height);
	}

	GetFirstComponentByType<Animator>()->SetPlaySpeed(mWalkSpeed);
}

bool SGGame::Zombie::canWalk()
{
	return 
		InputManager::Instance()->GetKeyHold('W') ||
		InputManager::Instance()->GetKeyHold('S') ||
		InputManager::Instance()->GetKeyHold('A') ||
		InputManager::Instance()->GetKeyHold('D') ||
		fabs(InputManager::Instance()->GetAxisValue(0, GamepadXinput::GamepadAxis::PadLX)) > WALK_THRESHOLD ||
		fabs(InputManager::Instance()->GetAxisValue(0, GamepadXinput::GamepadAxis::PadLY)) > WALK_THRESHOLD;
}

}