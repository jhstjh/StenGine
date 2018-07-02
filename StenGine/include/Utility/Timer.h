#ifndef __TIMER__
#define __TIMER__

#include "Graphics/D3DIncludes.h"
#include <windows.h>

namespace StenGine
{

class Timer {
private:
	static double m_gameStartTime;
	static double m_lastUpdateTime;
	static double m_deltaTime;
	static LARGE_INTEGER m_frequency;
	static LARGE_INTEGER m_counter;

public:
	Timer();
	~Timer();
	static void Init();
	static double GetTime();
	static double GetTimeSinceGameStart();
	static double GetDeltaTime() { return m_deltaTime; }
	static void Update();
};

}
#endif