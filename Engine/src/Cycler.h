#pragma once
#include "CommonStructures.h"

class Cycler
{
private:

	float m_timer;
	float m_relativeTime;
	CyclePeriod m_timePeriod = CyclePeriod::DAY;
	bool m_changedPeriod;

	float m_cycleSpeed;

	CyclePeriod CalculatePeriod();

public:

	Cycler();
	~Cycler() = default;

	void Update(float dt);

	CyclePeriod GetTimePeriod() const;
	
	bool HasChangedPeriod() const;

	float GetTime() const;
	void SetTime(float time);

	float GetCycleSpeed() const;
	void SetCycleSpeed(float speed);


};