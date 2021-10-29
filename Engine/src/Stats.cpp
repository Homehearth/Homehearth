#include "EnginePCH.h"
#include "Stats.h"

Stats::Stats()
{
	m_deltaTime	= 0.0f;
	m_maxFPS	= 140.0f;
}

const float& Stats::IGetDeltaTime() const
{
	return m_deltaTime;
}

void Stats::ISetDeltaTime(const float& dt)
{
	m_deltaTime = dt;
}

const float Stats::IGetCurrentFPS() const
{
	return 1.0f / m_deltaTime;
}

const float& Stats::IGetMaxFPS() const
{
	return m_maxFPS;
}

