#pragma once

#include <Xinput.h>

namespace StenGine
{

class GamepadXinput
{
public:
	enum GamepadButton
	{
		PadDpadUp = XINPUT_GAMEPAD_DPAD_UP,
		PadDpadDown = XINPUT_GAMEPAD_DPAD_DOWN,
		PadDpadLeft = XINPUT_GAMEPAD_DPAD_LEFT,
		PadDpadRight = XINPUT_GAMEPAD_DPAD_RIGHT,
		PadStart = XINPUT_GAMEPAD_START,
		PadBack = XINPUT_GAMEPAD_BACK,
		PadLeftThumb = XINPUT_GAMEPAD_LEFT_THUMB,
		PadRightThumb = XINPUT_GAMEPAD_RIGHT_THUMB,
		PadLeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
		PadRightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
		PadButtonA = XINPUT_GAMEPAD_A,
		PadButtonB = XINPUT_GAMEPAD_B,
		PadButtonX = XINPUT_GAMEPAD_X,
		PadButtonY = XINPUT_GAMEPAD_Y,
	};

	enum GamepadAxis
	{
		PadLT,
		PadRT,
		PadLX,
		PadLY,
		PadRX,
		PadRY
	};

	void Update();

	bool GetButtonDown(uint32_t index, GamepadButton button);
	bool GetButtonUp(uint32_t index, GamepadButton button);
	bool GetButtonHold(uint32_t index, GamepadButton button);

	float GetAxisValue(uint32_t index, GamepadAxis axis);

private:
	void ProcessDeadZone();

	XINPUT_STATE mState[XUSER_MAX_COUNT]{};
	XINPUT_STATE mPrevState[XUSER_MAX_COUNT]{};
	bool mConnected[XUSER_MAX_COUNT]{};
};


}