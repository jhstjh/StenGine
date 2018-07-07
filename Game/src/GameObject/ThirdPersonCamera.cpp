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
		
	}

	void ThirdPersonCamera::Update()
	{
		GetTransform()->LookAt({ 0.f, 15.f, 0.f }, { 0.f, 1.f, 0.f });
	}
}