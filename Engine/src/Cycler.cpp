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
	, blackboard(nullptr)
{
}

void Cycler::Update(float dt)
{
		m_timer += dt * m_cycleSpeed;
	if (m_timer > DAY_DURATION)
	{
		m_timer -= DAY_DURATION;
	}
	m_relativeTime = m_timer / DAY_DURATION;

	m_changedPeriod = false;

	if (blackboard)
	{
		CyclePeriod newPeriod = this->CalculatePeriod();
		if (newPeriod != m_timePeriod)
		{
			m_changedPeriod = true;

			blackboard->AddValue<CyclePeriod>("cycle", newPeriod);

			
		}
		m_timePeriod = newPeriod;
	}
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

float Cycler::GetDefaultSpeed() const
{
	return m_defaultCycleSpeed;
}

void Cycler::SetTimePeriod(CyclePeriod period, bool hasChangedPeriod)
{
	if (hasChangedPeriod)
	{
		m_timePeriod = period;
		m_changedPeriod = true;
	}
}

void Cycler::SetCycleSpeed(float speed)
{
	m_cycleSpeed = speed;
}

void Cycler::setBlackboard(Blackboard* blackboard)
{
	this->blackboard = blackboard;
}

void Cycler::ResetCycleSpeed()
{
	this->m_cycleSpeed = m_defaultCycleSpeed;
}
