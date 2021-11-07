#include "EnginePCH.h"
#include "Stats.h"

Stats::Stats()
{
	m_deltaTime		= 0.0f;
	m_frameTime		= 0.0f;
	m_updateTime	= 0.0f;
	m_networkTime	= 0.0f;

	m_framerate		= 120.0f;
	m_updaterate	= 1000.0f;
	m_tickrate		= 60.0f;
}

void Stats::SetDeltaTime(const float& dt)
{
	m_deltaTime = dt;
}
const float& Stats::GetDeltaTime() const
{
	return m_deltaTime;
}


void Stats::SetFrameTime(const float& ft)
{
	m_frameTime = ft;
}
const float& Stats::GetFrameTime() const
{
	return m_frameTime;
}


void Stats::SetUpdateTime(const float& ut)
{
	m_updateTime = ut;
}
const float& Stats::GetUpdateTime() const
{
	return m_updateTime;
}


void Stats::SetNetworkTime(const float& nt)
{
	m_networkTime = nt;
}
const float& Stats::GetNetworkTime() const
{
	return m_networkTime;
}


const float& Stats::GetFramerate() const
{
	return m_framerate;
}
const float& Stats::GetUpdaterate() const
{
	return m_updaterate;
}
const float& Stats::GetTickrate() const
{
	return m_tickrate;
}

