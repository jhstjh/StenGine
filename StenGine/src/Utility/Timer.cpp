#include "Utility/Timer.h"

float Timer::m_gameStartTime = 0;
float Timer::m_lastUpdateTime = 0;
float Timer::m_deltaTime = 0;
LARGE_INTEGER Timer::m_frequency = { 0 };
LARGE_INTEGER Timer::m_counter = { 0 };

void Timer::Init() {
	QueryPerformanceFrequency(&m_frequency);
	m_gameStartTime = GetTime();
	m_lastUpdateTime = GetTime();
}

float Timer::GetTime() {
	QueryPerformanceCounter(&m_counter); 
	return (float)(m_counter.QuadPart) / (float)m_frequency.QuadPart;
}

float Timer::GetTimeSinceGameStart() {
	return GetTime() - m_gameStartTime;
}

void Timer::Update() {
	float time = GetTime();
	m_deltaTime = time - m_lastUpdateTime;
	m_lastUpdateTime = time;
}

Timer::~Timer(){
}