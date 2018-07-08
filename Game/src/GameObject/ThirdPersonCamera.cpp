#include "GameObject/ThirdPersonCamera.h"
#include "Scene/CameraManager.h"
#include "Scene/GameObject.h"
#include "Scene/GameObjectManager.h"
#include "Input/InputManager.h"
#include "Math/MathHelper.h"

namespace SGGame
{
	ThirdPersonCamera::ThirdPersonCamera()
	{

	}

	void ThirdPersonCamera::Start()
	{
		Vec3 pos = GetTransform()->GetPosition();

		mRadius = pos.Length();
		mTheta = fmod(atan2f(pos.z(), pos.x()), 2 * PI);
		mPhi = acosf(pos.y() / mRadius);

		GetTransform()->LookAt({ 0.f, 15.f, 0.f }, { 0.f, 1.f, 0.f });
	}

	void ThirdPersonCamera::Update()
	{
		auto dt = Timer::GetDeltaTime();
		const float rotSpeed{ 2.f };

		float diffTheta = 0.f;
		float diffPhi = 0.f;
		if (InputManager::Instance()->GetKeyHold(VK_UP))
		{
			diffPhi += rotSpeed * dt;
		}
		if (InputManager::Instance()->GetKeyHold(VK_DOWN))
		{
			diffPhi -= rotSpeed * dt;
		}
		if (InputManager::Instance()->GetKeyHold(VK_LEFT))
		{
			diffTheta += rotSpeed * dt;
		}
		if (InputManager::Instance()->GetKeyHold(VK_RIGHT))
		{
			diffTheta -= rotSpeed * dt;
		}

		if (diffTheta != 0.f || diffPhi != 0.f)
		{
			mTheta += diffTheta;
			mPhi += diffPhi;

			if (mPhi > PI) mPhi = PI;
			if (mPhi < 0.f) mPhi = 0.f;

			float x = mRadius * cosf(mTheta) * sinf(mPhi);
			float y = mRadius * cosf(mPhi);
			float z = mRadius * sinf(mTheta) * sinf(mPhi);

			auto trans = GetTransform();
			trans->SetPosX(x);
			trans->SetPosY(y);
			trans->SetPosZ(z);

			trans->LookAt({ 0.f, 15.f, 0.f }, { 0.f, 1.f, 0.f });
		}
	}

}