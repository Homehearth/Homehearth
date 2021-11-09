#pragma once
#include <unordered_map>
#include "Components.h"
#include "Scene.h"

class Predictor
{
private:

	std::unordered_map<uint32_t, std::pair<sm::Vector3, uint32_t>> m_networkPositions;
	std::unordered_map < uint32_t, std::pair<sm::Vector3, uint32_t>> m_predictedVectors;

	/*
		t1 = current position in ecs system.
		t2 = newest position from network.
	*/
	void DoPolation(const uint32_t& id, comp::Transform& t1);

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
		Predict the new positions based on network data and local data.
	*/
	void Predict(Scene& scene);
};