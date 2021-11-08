#include "EnginePCH.h"
#include "Predictor.h"
#include "Stats.h"

void Predictor::DoExtrapolation(comp::Transform& t1, const comp::Transform& t2)
{

}

void Predictor::Add(const uint32_t& id, const comp::Transform& position)
{
	m_networkPositions[id] = position;
}

void Predictor::LinearExtrapolate(Scene& scene)
{
	scene.ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& net, comp::Transform& t){

		DoExtrapolation(t, m_networkPositions[net.id]);

		});
}
