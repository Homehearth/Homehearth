#pragma once
#include "Timer.h"
#include "CommonStructures.h"


class Cycler
{
private:

	Timer m_time;
	Cycle m_timePeriod = Cycle::DAY;
	bool m_shouldSwitch = false;

	void StartDay();
	void StartNight();
public:

	Cycler();
	~Cycler() = default;

	void OnStart();
	void Update();

	const Cycle& GetTimePeriod() const;
	const bool& GetSwitch() const;
	uint32_t GetElapsedTime() const;
	void Switch();
};