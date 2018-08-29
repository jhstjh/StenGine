#pragma once
#include <bitset>

namespace StenGine
{

class Keyboard
{
public:
	void Update();

	bool GetKeyDown(char key);
	bool GetKeyUp(char key);
	bool GetKeyHold(char key);

private:
	std::bitset<256> mKeyState;
	std::bitset<256> mLastKeyState;
};

}