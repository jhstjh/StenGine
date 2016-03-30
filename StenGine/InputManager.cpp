#include "InputManager.h"


InputManager* InputManager::_instance = nullptr;

InputManager::InputManager() 
{

}

InputManager::~InputManager() {

}

void InputManager::Update() {
	m_lastKeyState = m_keyState;
	for (int x = 0; x < 256; x++)
		m_keyState[x] = ((USHORT)(GetAsyncKeyState(x)) >> 15) > 0;
}

bool InputManager::GetKeyDown(char key) {
	if (m_keyState[(int)key]  && !m_lastKeyState[(int)key]) {
		return true;
	}
	return false;
}

bool InputManager::GetKeyUp(char key) {
	if (m_lastKeyState[(int)key] && !m_keyState[(int)key]) {
		return true;
	}
	return false;
}

bool InputManager::GetKeyHold(char key) {
	if (m_keyState[(int)key] && m_lastKeyState[(int)key]) {
		return true;
	}
	return false;
}