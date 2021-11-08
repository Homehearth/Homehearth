#include "EnginePCH.h"
#include "Predictor.h"
#include "Stats.h"

void Predictor::DoExtrapolation(const uint32_t& id, const comp::Transform& t1, const comp::Transform& t2)
{
	// Set it to act as current position.
	comp::Transform newT = t1;

	// Take out the last predicted position.
	comp::Transform t3 = m_networkPositions.at(id);

	// Do linear extrapolation on positions.
	newT.position.x = t1.position.x + (t2.position.z - t1.position.z) / (t3.position.z - t1.position.z) * (t3.position.x - t1.position.x);
	newT.position.z = t1.position.z + (t2.position.x - t1.position.x) / (t3.position.x - t1.position.x) * (t3.position.z - t1.position.z);

	// Save the newely predicted position.
	m_networkPositions[id] = newT;
}

void Predictor::Add(const uint32_t& id, const comp::Transform& position)
{
	m_networkPositions[id] = position;

	// If position is added for the first time add it to predicted positions too.
	if (m_predictedPositions.find(id) != m_predictedPositions.end())
	{
		m_predictedPositions[id] = position;
	}
}

void Predictor::LinearExtrapolate(Scene& scene)
{
	scene.ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& net, comp::Transform& t){

		DoExtrapolation(net.id, t, m_networkPositions[net.id]);

		});
}
