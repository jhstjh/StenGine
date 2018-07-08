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
};

}