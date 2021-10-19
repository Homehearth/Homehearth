#pragma once
#include "Scene.h"
namespace Systems
{
	void MovementSystem(HeadlessScene& scene, float dt);
	void MovementColliderSystem(HeadlessScene& scene, float dt);
	void LightSystem(Scene& scene, float dt);
}
