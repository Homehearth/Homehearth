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
	const float m_defaultCycleSpeed = 1.f;

	CyclePeriod CalculatePeriod();
	Blackboard* blackboard;

public:

	Cycler();
	~Cycler() = default;

	void Update(float dt, HeadlessScene& scene);
	void Update(float dt, Scene& scene);

	CyclePeriod GetTimePeriod() const;
	
	bool HasChangedPeriod() const;

	float GetTime() const;
	void SetTime(float time);

	float GetCycleSpeed() const;
	float GetDefaultSpeed() const;
	void SetTimePeriod(CyclePeriod period, bool hasChangedPeriod);
	void SetCycleSpeed(float speed);
	void setBlackboard(Blackboard* blackboard);

	void ResetCycleSpeed();

};