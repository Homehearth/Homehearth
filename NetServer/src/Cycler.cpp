#include "NetServerPCH.h"
#include "Cycler.h"
#include "Simulation.h"

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

void Cycler::Update(Simulation* sim)
{
	switch (m_timePeriod)
	{
	case Cycle::DAY:
	{
		this->OnDay(sim);
		uint32_t elapsed = (uint32_t)m_time.GetElapsedTime();

		// The day has reached its limit and switches to night.
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
		// Change to day when no more enemies are present.
		if (this->OnNight(sim) == 0)
		{
			m_timePeriod = Cycle::DAY;
			m_time.Start();
		}

		uint32_t elapsed = (uint32_t)m_time.GetElapsedTime();

		// The night has reached its limit and switches to day.
		if (elapsed >= TIME_LIMIT_NIGHT)
		{
			m_timePeriod = Cycle::MORNING;
			m_time.Start();
		}
		break;
	}
	case Cycle::MORNING:
	{
		uint32_t elapsed = (uint32_t)m_time.GetElapsedTime();
		if (elapsed >= TIME_LIMIT_MORNING)
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

void Cycler::OnDay(Simulation* sim)
{
	// Remove any baddies on the day.
	sim->GetGameScene()->ForEachComponent<comp::Tag<TagType::BAD>>([](Entity e, comp::Tag<TagType::BAD>) {

		e.Destroy();

		});
}

int Cycler::OnNight(Simulation* sim)
{
	int count = 0;
	// Remove any baddies on the day.
	sim->GetGameScene()->ForEachComponent<comp::Tag<TagType::BAD>>([&](Entity e, comp::Tag<TagType::BAD>) {
		count++;
		});

	return count;
}

Cycle Cycler::GetTimePeriod() const
{
	return m_timePeriod;
}

bool Cycler::GetSwitch() const
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
