#ifndef __TIMER__
#define __TIMER__

#include "Graphics/D3DIncludes.h"
#include <windows.h>

namespace StenGine
{

class Timer {
public:
	using Seconds = float;

	Timer();
	~Timer();
	static void Init();
	static Seconds GetTime();
	static Seconds GetTimeSinceGameStart();
	static Seconds GetDeltaTime() { return m_deltaTime; }
	static void Update();
private:
	static Seconds m_gameStartTime;
	static Seconds m_lastUpdateTime;
	static Seconds m_deltaTime;
	static LARGE_INTEGER m_frequency;
	static LARGE_INTEGER m_counter;
};

}
#endif