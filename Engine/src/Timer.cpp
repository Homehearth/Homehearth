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
	m_startTime = std::chrono::steady_clock::now();
	m_hasStoped = false;
}





//--------------------------------------------------------------------------------------
void Timer::Stop()
{
	m_stopTime = std::chrono::steady_clock::now();
	m_hasStoped = true;
}





//--------------------------------------------------------------------------------------
double Timer::GetElapsedTime() const
{
	std::chrono::duration<double> elapsed_seconds;

	if(m_hasStoped)
		elapsed_seconds = m_stopTime - m_startTime;
	else
		elapsed_seconds = std::chrono::steady_clock::now() - m_startTime;

	return elapsed_seconds.count();
}
