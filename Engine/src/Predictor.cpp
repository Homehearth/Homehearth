#include "EnginePCH.h"
#include "Predictor.h"
#include "Stats.h"

const static sm::Vector3 NULL_VECTOR = { 0.0f, 0.0f, 0.0f };

void Predictor::DoPolation(const uint32_t& id, comp::Transform& t1)
{

}

void Predictor::Add(const uint32_t& id, const comp::Transform& position)
{

}

void Predictor::Remove(const uint32_t& id)
{

}

void Predictor::Predict(Scene& scene)
{
	scene.ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& net, comp::Transform& t){

		});
}
