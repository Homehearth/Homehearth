#pragma once
#include <unordered_map>
#include "Components.h"
#include "Scene.h"

class Predictor
{
private:

	std::unordered_map<uint32_t, comp::Transform> m_networkPositions;
	std::unordered_map<uint32_t, comp::Transform> m_oldPositions;

	/*
		t1 = current position in ecs system.
		t2 = newest position from network.
	*/
	void DoExtrapolation(const uint32_t& id, comp::Transform& t1, const comp::Transform& t2);

public:

	Predictor() = default;
	~Predictor() = default;

	/*
		Add or Update a position in the map.
	*/
	void Add(const uint32_t& id, const comp::Transform& position);

	/*
		Remove id from map
	*/
	void Remove(const uint32_t& id);

	/*
		Perform linear extrapolation on the chosen scene with the available positions.
	*/
	void LinearExtrapolate(Scene& scene);
};