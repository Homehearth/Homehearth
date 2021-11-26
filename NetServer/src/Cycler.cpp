#include "NetServerPCH.h"
#include "Cycler.h"

void Cycler::StartDay()
{
	m_timePeriod = Cycle::DAY;
}

void Cycler::StartNight()
{
	m_timePeriod = Cycle::NIGHT;
}

Cycler::Cycler()
{
}

void Cycler::OnStart()
{
	m_time.Start();
	m_timePeriod = Cycle::DAY;
}

void Cycler::Update()
{
	switch (m_timePeriod)
	{
	case Cycle::DAY:
	{
		uint32_t elapsed = (uint32_t)m_time.GetElapsedTime();
		//LOG_INFO("Day Time elapsed: %lu", elapsed);
		if (elapsed >= TIME_LIMIT_DAY)
		{
			m_timePeriod = Cycle::NIGHT;
			m_time.Start();
			m_shouldSwitch = true;
		}
		break;
	}
	case Cycle::NIGHT:
	{
		uint32_t elapsed = (uint32_t)m_time.GetElapsedTime();
		//LOG_INFO("Night Time elapsed: %lu", elapsed);
		if (elapsed >= TIME_LIMIT_NIGHT)
		{
			m_timePeriod = Cycle::DAY;
			m_time.Start();
		}
		break;
	}
	default:
		break;
	}
}

const Cycle& Cycler::GetTimePeriod() const
{
	return m_timePeriod;
}

const bool& Cycler::GetSwitch() const
{
	return m_shouldSwitch;
}

uint32_t Cycler::GetElapsedTime() const
{
	return (uint32_t)m_time.GetElapsedTime();
}

void Cycler::Switch()
{
	m_shouldSwitch = false;
}
