#pragma once

class Simulation;

constexpr float SPREE_TIME = 3.0f;

class SpreeHandler
{
private:

	unsigned int m_currentSpree = 1;
	float m_timer = 0.0f;

public:

	SpreeHandler();
	~SpreeHandler() = default;

	void Update();
	const unsigned int& GetSpree() const;
	void AddSpree();
};