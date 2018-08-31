#include "stdafx.h"

#include "Input/InputManager.h"
#include "Engine/EventSystem.h"

namespace StenGine
{

DEFINE_SINGLETON_CLASS(InputManager)

InputManager::InputManager()
{
	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE_INPUT, [this](){Update();});
}

InputManager::~InputManager() {

}

void InputManager::Update() {
	mKeyboard.Update();
	mGamepad.Update();
}

bool InputManager::GetKeyDown(char key) {
	return mKeyboard.GetKeyDown(key);
}

bool InputManager::GetKeyUp(char key) {
	return mKeyboard.GetKeyUp(key);
}

bool InputManager::GetKeyHold(char key) {
	return mKeyboard.GetKeyHold(key);
}

bool InputManager::GetButtonDown(uint32_t index, GamepadXinput::GamepadButton button)
{
	return mGamepad.GetButtonDown(index, button);
}

bool InputManager::GetButtonUp(uint32_t index, GamepadXinput::GamepadButton button)
{
	return mGamepad.GetButtonUp(index, button);
}

bool InputManager::GetButtonHold(uint32_t index, GamepadXinput::GamepadButton button)
{
	return mGamepad.GetButtonHold(index, button);
}

float InputManager::GetAxisValue(uint32_t index, GamepadXinput::GamepadAxis axis)
{
	return mGamepad.GetAxisValue(index, axis);
}

}