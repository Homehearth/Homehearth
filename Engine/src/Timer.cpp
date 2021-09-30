#include "EnginePCH.h"

#include "Timer.h"

//--------------------------------------------------------------------------------------
Timer::Timer()
	: m_hasStoped(false)
{
	Start();
}





//--------------------------------------------------------------------------------------
void Timer::Start()
{
	m_startTime = std::chrono::high_resolution_clock::now();
	m_hasStoped = false;
}





//--------------------------------------------------------------------------------------
void Timer::Stop()
{
	m_stopTime = std::chrono::high_resolution_clock::now();
	m_hasStoped = true;
}




