#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__
//#include <Xinput.h>
#include "Graphics/D3DIncludes.h"
#include "System/SingletonClass.h"
#include <bitset>

namespace StenGine
{

class InputManager : public SingletonClass<InputManager>
{
private:

	//  	struct CONTROLLER_STATE
	//  	{
	//  		XINPUT_STATE lastState;
	//  		XINPUT_STATE state;
	//  		DWORD dwResult;
	//  		bool bLockVibration;
	//  		XINPUT_VIBRATION vibration;
	//  	} m_controller;

	std::bitset<256> m_keyState;
	std::bitset<256> m_lastKeyState;

public:
	InputManager();
	~InputManager();

	void Update();
	bool GetKeyDown(char key);
	bool GetKeyUp(char key);
	bool GetKeyHold(char key);
};

}
#endif // !__XINPUT_MANAGER__
