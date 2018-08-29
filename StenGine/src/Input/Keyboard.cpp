#include <Windows.h>
#include "Input/Keyboard.h"

namespace StenGine
{

void Keyboard::Update() {
	mLastKeyState = mKeyState;
	for (int x = 0; x < 256; x++)
		mKeyState[x] = ((USHORT)(GetAsyncKeyState(x)) >> 15) > 0;
}

bool Keyboard::GetKeyDown(char key) {
	if (mKeyState[(int)key] && !mLastKeyState[(int)key]) {
		return true;
	}
	return false;
}

bool Keyboard::GetKeyUp(char key) {
	if (mLastKeyState[(int)key] && !mKeyState[(int)key]) {
		return true;
	}
	return false;
}

bool Keyboard::GetKeyHold(char key) {
	if (mKeyState[(int)key] && mLastKeyState[(int)key]) {
		return true;
	}
	return false;
}

}