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
	m_lastKeyState = m_keyState;
	for (int x = 0; x < 256; x++)
		m_keyState[x] = ((USHORT)(GetAsyncKeyState(x)) >> 15) > 0;
}

bool InputManager::GetKeyDown(char key) {
	if (m_keyState[(int)key] && !m_lastKeyState[(int)key]) {
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

}