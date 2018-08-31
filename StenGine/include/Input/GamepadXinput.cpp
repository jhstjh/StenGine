#include "stdafx.h"

#include "GamepadXinput.h"

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30

namespace StenGine
{

void GamepadXinput::Update()
{
	DWORD dwResult;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		mPrevState[i] = mState[i];
		ZeroMemory(&mState[i], sizeof(XINPUT_STATE));

		dwResult = XInputGetState(i, &mState[i]);

		if (dwResult == ERROR_SUCCESS)
		{
			mConnected[i] = true;
			ProcessDeadZone();
		}
		else
		{
			mConnected[i] = false;
		}
	}
}

bool GamepadXinput::GetButtonDown(uint32_t index, GamepadButton button) {
	if ((mState[index].Gamepad.wButtons & button) != 0 && (mPrevState[index].Gamepad.wButtons & button) == 0) {
		return true;
	}
	return false;
}

bool GamepadXinput::GetButtonUp(uint32_t index, GamepadButton button) {
	if ((mState[index].Gamepad.wButtons & button) == 0 && (mPrevState[index].Gamepad.wButtons & button) != 0) {
		return true;
	}
	return false;
}

bool GamepadXinput::GetButtonHold(uint32_t index, GamepadButton button) {
	if ((mState[index].Gamepad.wButtons & button) != 0 && (mPrevState[index].Gamepad.wButtons & button) != 0) {
		return true;
	}
	return false;
}

float GamepadXinput::GetAxisValue(uint32_t index, GamepadAxis axis)
{
	switch (axis)
	{
	case PadLT:
		return mState[index].Gamepad.bLeftTrigger / 255.f;
	case PadRT:
		return mState[index].Gamepad.bRightTrigger / 255.f;
	case PadLX:
		return mState[index].Gamepad.sThumbLX / 32768.f;
	case PadLY:
		return mState[index].Gamepad.sThumbLY / 32768.f;
	case PadRX:
		return mState[index].Gamepad.sThumbRX / 32768.f;
	case PadRY:
		return mState[index].Gamepad.sThumbRY / 32768.f;
	}

	return 0.f;
}

void GamepadXinput::ProcessDeadZone()
{
	// TODO
}

}