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
		auto targetObject = GameObjectManager::Instance()->FindGameObjectByName("Zombie");
		assert(targetObject);

		mTarget = targetObject->GetTransform();

		Vec3 pos = GetTransform()->GetPosition();
		Vec3 targetPos = mTarget->GetPosition();
		mPrevTargetPos = targetPos;
		Vec3 diff = pos - targetPos;

		Vec3 offset = diff.Normalized() * DISTANCE;
		Vec3 newPos = targetPos + offset;
		GetTransform()->SetPosX(newPos.x());
		GetTransform()->SetPosY(newPos.y());
		GetTransform()->SetPosZ(newPos.z());

		pos = GetTransform()->GetPosition();

		mRadius = offset.Length();
		mTheta = fmod(atan2f(offset.z(), offset.x()), 2 * PI);
		mPhi = acosf(offset.y() / mRadius);

		GetTransform()->LookAt(targetPos + HEIGHT_OFFSET, { 0.f, 1.f, 0.f });
	}

	void ThirdPersonCamera::Update()
	{
		auto dt = Timer::GetDeltaTime();
		Vec3 targetPos = mTarget->GetPosition();
		bool dirtyLookAt = false;

		if ((targetPos - mPrevTargetPos).LengthSquared() > 0.f)
		{
			Vec3 pos = GetTransform()->GetPosition();
			Vec3 diff = pos - targetPos;

			float scale = 1.f;
			float sqrDiff = diff.LengthSquared() - DISTANCE * DISTANCE;

			if (sqrDiff > 0.f)
			{
				scale = sqrt(sqrDiff / (diff.x() * diff.x() + diff.z() * diff.z()) + 1);
			}
			else
			{
				scale = sqrt(1 - -sqrDiff / (diff.x() * diff.x() + diff.z() * diff.z()));
			}

			diff.x() /= scale;
			diff.z() /= scale;

			Vec3 newPos = targetPos + diff;
			GetTransform()->SetPosX(newPos.x());
			GetTransform()->SetPosY(newPos.y());
			GetTransform()->SetPosZ(newPos.z());

			mRadius = diff.Length();
			mTheta = fmod(atan2f(diff.z(), diff.x()), 2 * PI);
			mPhi = acosf(diff.y() / mRadius);

			dirtyLookAt = true;
		}		
		
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

			if (mPhi >= PI) mPhi = PI - 0.0001f;
			if (mPhi < 0.f) mPhi = 0.f;

			float x = mRadius * cosf(mTheta) * sinf(mPhi) + targetPos.x();
			float y = mRadius * cosf(mPhi) + targetPos.y();
			float z = mRadius * sinf(mTheta) * sinf(mPhi) + targetPos.z();

			auto trans = GetTransform();
			trans->SetPosX(x);
			trans->SetPosY(y);
			trans->SetPosZ(z);

			dirtyLookAt = true;
		}

		if (dirtyLookAt)
		{
			GetTransform()->LookAt(targetPos + HEIGHT_OFFSET, { 0.f, 1.f, 0.f });
		}

		mPrevTargetPos = targetPos;
	}
}