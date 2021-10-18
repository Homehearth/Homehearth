#pragma once
#include "Scene.h"
namespace Systems
{
	void MovementSystem(HeadlessScene& scene, float dt);
	void UpdateColliderPosSystem(HeadlessScene& scene, float dt);
}
