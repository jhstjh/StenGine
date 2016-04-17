#ifndef __TIMER__
#define __TIMER__

#include "Graphics/D3DIncludes.h"
#include <windows.h>

namespace StenGine
{

class Timer {
private:
	static float m_gameStartTime;
	static float m_lastUpdateTime;
	static float m_deltaTime;
	static LARGE_INTEGER m_frequency;
	static LARGE_INTEGER m_counter;

public:
	Timer();
	~Timer();
	static void Init();
	static float GetTime();
	static float GetTimeSinceGameStart();
	static float GetDeltaTime() { return m_deltaTime; }
	static void Update();
};

}
#endif