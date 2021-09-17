#include "EnginePCH.h"

#include "Timer.h"

//--------------------------------------------------------------------------------------
Timer::Timer()
	: hasStoped(false)
{
	start();
}





//--------------------------------------------------------------------------------------
void Timer::start()
{
	startTime = std::chrono::steady_clock::now();
	hasStoped = false;
}





//--------------------------------------------------------------------------------------
void Timer::stop()
{
	stopTime = std::chrono::steady_clock::now();
	hasStoped = true;
}





//--------------------------------------------------------------------------------------
double Timer::getElapsedTime() const
{
	std::chrono::duration<double> elapsed_seconds;

	if(hasStoped)
		elapsed_seconds = stopTime - startTime;
	else
		elapsed_seconds = std::chrono::steady_clock::now() - startTime;

	return elapsed_seconds.count();
}
