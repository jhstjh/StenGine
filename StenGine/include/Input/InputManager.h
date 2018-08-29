#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__
#include "GamepadXinput.h"
#include "Keyboard.h"
#include "Graphics/D3DIncludes.h"
#include "System/SingletonClass.h"

namespace StenGine
{

class InputManager : public SingletonClass<InputManager>
{
public:
	InputManager();
	~InputManager();

	void Update();
	bool GetKeyDown(char key);
	bool GetKeyUp(char key);
	bool GetKeyHold(char key);

	bool GetButtonDown(uint32_t index, GamepadXinput::GamepadButton button);
	bool GetButtonUp(uint32_t index, GamepadXinput::GamepadButton button);
	bool GetButtonHold(uint32_t index, GamepadXinput::GamepadButton button);

	float GetAxisValue(uint32_t index, GamepadXinput::GamepadAxis axis);

private:
	Keyboard mKeyboard;
	GamepadXinput mGamepad;
};

}
#endif // !__XINPUT_MANAGER__
