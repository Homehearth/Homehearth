#pragma once
#include "CommonStructures.h"

class Cycler
{
private:

	float m_timer;
	float m_relativeTime;
	CyclePeriod m_timePeriod = CyclePeriod::DAY;

	CyclePeriod CalculatePeriod();

public:

	Cycler();
	~Cycler() = default;

	void Update(float dt);

	CyclePeriod GetTimePeriod() const;
	
	float GetTime() const;
	void SetTime(float time);

};