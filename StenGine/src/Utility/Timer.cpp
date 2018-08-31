#include "stdafx.h"

#include "Utility/Timer.h"

namespace StenGine
{

Timer::Seconds Timer::m_gameStartTime = 0;
Timer::Seconds Timer::m_lastUpdateTime = 0;
Timer::Seconds Timer::m_deltaTime = 0;
LARGE_INTEGER Timer::m_frequency = { 0 };
LARGE_INTEGER Timer::m_counter = { 0 };

void Timer::Init() {
	QueryPerformanceFrequency(&m_frequency);
	m_gameStartTime = GetTime();
	m_lastUpdateTime = GetTime();
}

Timer::Seconds Timer::GetTime() {
	QueryPerformanceCounter(&m_counter);
	return (float)((double)(m_counter.QuadPart) / (double)m_frequency.QuadPart);
}

Timer::Seconds Timer::GetTimeSinceGameStart() {
	return GetTime() - m_gameStartTime;
}

void Timer::Update() {
	Seconds time = GetTime();
	m_deltaTime = time - m_lastUpdateTime;
	m_lastUpdateTime = time;
}

Timer::~Timer() {
}

}