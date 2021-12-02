#include "EnginePCH.h"
#include "Cycler.h"

CyclePeriod Cycler::CalculatePeriod()
{
	if (m_relativeTime > NIGHT)
		return CyclePeriod::NIGHT;
	
	else if (m_relativeTime > EVENING)
		return CyclePeriod::EVENING;

	else if (m_relativeTime > DAY)
		return CyclePeriod::DAY;
	

	return CyclePeriod::MORNING;
}

Cycler::Cycler()
	: m_timer(0.0f)
	, m_relativeTime(0.0f)
	, m_timePeriod(CyclePeriod::DAY)
	, m_changedPeriod(false)
	, m_cycleSpeed(1.0f)
{
	Blackboard::Get().AddValue<CyclePeriod>("cycle", m_timePeriod);
}

void Cycler::Update(float dt)
{
	m_timer += dt * m_cycleSpeed;
	if (m_timer > DAY_DURATION)
	{
		m_timer = 0.0f;
	}
	m_relativeTime = m_timer / DAY_DURATION;

	CyclePeriod newPeriod = this->CalculatePeriod();
	m_changedPeriod = false;
	if (newPeriod != m_timePeriod)
	{
		m_changedPeriod = true;

		Blackboard::Get().AddValue<CyclePeriod>("cycle", newPeriod);
	}
	m_timePeriod = newPeriod;
}

CyclePeriod Cycler::GetTimePeriod() const
{
	return m_timePeriod;
}

bool Cycler::HasChangedPeriod() const
{
	return m_changedPeriod;
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

float Cycler::GetCycleSpeed() const
{
	return m_cycleSpeed;
}

void Cycler::SetCycleSpeed(float speed)
{
	m_cycleSpeed = speed;
}