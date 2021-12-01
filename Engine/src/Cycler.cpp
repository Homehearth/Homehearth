#include "EnginePCH.h"
#include "Cycler.h"

CyclePeriod Cycler::CalculatePeriod()
{
	if (m_relativeTime > NIGHT)
	{
		return CyclePeriod::NIGHT;
	}
	else if (m_relativeTime > DAY)
	{
		return CyclePeriod::DAY;
	}

	return CyclePeriod::MORNING;
}

Cycler::Cycler()
	: m_timer(0.0f)
	, m_relativeTime(0.0f)
	, m_timePeriod(CyclePeriod::DAY)
{
}

void Cycler::Update(float dt)
{
	m_timer += dt;
	if (m_timer > DAY_DURATION)
	{
		m_timer = 0.0f;
	}
	m_relativeTime = m_timer / DAY_DURATION;

	m_timePeriod = this->CalculatePeriod();
}

CyclePeriod Cycler::GetTimePeriod() const
{
	return m_timePeriod;
}

float Cycler::GetTime() const
{
	return m_relativeTime;
}

void Cycler::SetTime(float time)
{
	m_timer = time * DAY_DURATION;
	m_relativeTime = time;
	m_timePeriod = this->CalculatePeriod();
}
