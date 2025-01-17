#include "stdafx.h"

#include "GameObject/DebugCamera.h"
#include "Scene/CameraManager.h"
#include "Input/InputManager.h"
#include "Math/MathHelper.h"

namespace SGGame
{
	DebugCamera::DebugCamera()
	{

	}

	void DebugCamera::Update()
	{
		static const float MOVE_SPEED{ 10.f };

		auto transform = GetTransform();
		if (InputManager::Instance()->GetKeyHold('W'))
		{
			transform->MoveForward(MOVE_SPEED * Timer::GetDeltaTime());
		}
		if (InputManager::Instance()->GetKeyHold('S'))
		{
			transform->MoveBack(MOVE_SPEED * Timer::GetDeltaTime());
		}
		if (InputManager::Instance()->GetKeyHold('A'))
		{
			transform->MoveLeft(MOVE_SPEED * Timer::GetDeltaTime());
		}
		if (InputManager::Instance()->GetKeyHold('D'))
		{
			transform->MoveRight(MOVE_SPEED * Timer::GetDeltaTime());
		}
		if (InputManager::Instance()->GetKeyHold(VK_UP))
		{
			transform->Rotate(-PI / 3.0f * Timer::GetDeltaTime(), {1, 0, 0}, true);
		}
		if (InputManager::Instance()->GetKeyHold(VK_DOWN))
		{
			transform->Rotate(PI / 3.0f * Timer::GetDeltaTime(), { 1, 0, 0 }, true);
		}
		if (InputManager::Instance()->GetKeyHold(VK_LEFT))
		{
			transform->Rotate(-PI / 3.0f * Timer::GetDeltaTime(), { 0, 1, 0 }, false);
		}
		if (InputManager::Instance()->GetKeyHold(VK_RIGHT))
		{
			transform->Rotate(PI / 3.0f * Timer::GetDeltaTime(), { 0, 1, 0 }, false);
		}
	}
}