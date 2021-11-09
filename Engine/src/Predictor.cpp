#include "EnginePCH.h"
#include "Predictor.h"
#include "Stats.h"

void Predictor::DoExtrapolation(const uint32_t& id, comp::Transform& t1, const comp::Transform& t2)
{
	comp::Transform newT = t2;

	// Take out the last predicted position.
	comp::Transform t3 = m_oldPositions.at(id);

	// Do linear extrapolation on positions.
	newT.position.x = t1.position.x + ((t2.position.z - t1.position.z) / (t3.position.z - t1.position.z)) * (t3.position.x - t1.position.x);
	newT.position.y = t2.position.y;
	newT.position.z = t1.position.z + ((t2.position.x - t1.position.x) / (t3.position.x - t1.position.x)) * (t3.position.z - t1.position.z);

	// Take the difference between calculated position and current position.
	sm::Vector3 diff = newT.position - t1.position;

	// Save result to networkPosition.
	m_networkPositions[id] = newT;

	// Add the difference to current position with delta time factored in.
	/*
	t1.position.x += diff.x * Stats::GetDeltaTime();
	t1.position.z += diff.z * Stats::GetDeltaTime();
	*/
	t1.position = sm::Vector3::Lerp(t1.position, newT.position, Stats::GetDeltaTime());
	t1.rotation = t2.rotation;
	t1.scale = t2.scale;

	LOG_INFO("ID: %u, OldPos x: %f, y: %f, z: %f", id, t3.position.x, t3.position.y, t3.position.z);
	LOG_INFO("ID: %u, NetworkPos x: %f, y: %f, z: %f", id, t2.position.x, t2.position.y, t2.position.z);
	LOG_INFO("ID: %u, LocalPos x: %f, y: %f, z: %f", id, t1.position.x, t1.position.y, t1.position.z);
	LOG_INFO("ID: %u, PredictedPos x: %f, y: %f, z: %f", id, newT.position.x, newT.position.y, newT.position.z);
}

void Predictor::Add(const uint32_t& id, const comp::Transform& position)
{
	/*
		Add position to previous positions and update if available.
	*/
	if (m_networkPositions.find(id) != m_networkPositions.end())
	{
		m_oldPositions[id] = m_networkPositions[id];
	}

	// Update network position.
	m_networkPositions[id] = position;
	
}

void Predictor::Remove(const uint32_t& id)
{
	m_networkPositions.erase(id);
	m_oldPositions.erase(id);
}

void Predictor::LinearExtrapolate(Scene& scene)
{
	scene.ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& net, comp::Transform& t){

		auto it = m_oldPositions.find(net.id);
		if (it != m_oldPositions.end() && it->second.position != m_networkPositions.at(net.id).position && net.id != 0)
			DoExtrapolation(net.id, t, m_networkPositions[net.id]);
		});
}
