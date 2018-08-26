#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace StenGine
{
class Transform;
}

namespace SGGame
{

class ThirdPersonCamera : public GameObject
{
public:
	ThirdPersonCamera();

	virtual void Start() override;
	virtual void Update() override;

private:

	float		 mTheta;
	float		 mPhi;
	float		 mRadius;

	Transform*   mTarget{ nullptr };
	Vec3		 mPrevTargetPos;

	const float  DISTANCE{ 30.f };
	const Vec3   HEIGHT_OFFSET{ 0.f, 4.f, 0.f };
};

}