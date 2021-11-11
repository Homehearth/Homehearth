#include "EnginePCH.h"
#include "Stats.h"

Stats::Stats()
{
	m_frameTime		= 0.0f;
	m_updateTime	= 0.0f;
	m_networkTime	= 0.0f;

	m_framerate		= 144.0f;
	//High updaterate can cause a lot of stuttering
	m_updaterate	= 1000.0f;
	m_tickrate		= 60.0f;
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
void Stats::SetUpdateRate(const float& rate)
{
	m_updaterate = rate;
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

