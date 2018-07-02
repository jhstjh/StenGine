#include "Utility/Timer.h"

namespace StenGine
{

double Timer::m_gameStartTime = 0;
double Timer::m_lastUpdateTime = 0;
double Timer::m_deltaTime = 0;
LARGE_INTEGER Timer::m_frequency = { 0 };
LARGE_INTEGER Timer::m_counter = { 0 };

void Timer::Init() {
	QueryPerformanceFrequency(&m_frequency);
	m_gameStartTime = GetTime();
	m_lastUpdateTime = GetTime();
}

double Timer::GetTime() {
	QueryPerformanceCounter(&m_counter);
	return (double)(m_counter.QuadPart) / (double)m_frequency.QuadPart;
}

double Timer::GetTimeSinceGameStart() {
	return GetTime() - m_gameStartTime;
}

void Timer::Update() {
	double time = GetTime();
	m_deltaTime = time - m_lastUpdateTime;
	m_lastUpdateTime = time;
}

Timer::~Timer() {
}

}