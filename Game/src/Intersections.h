#pragma once
#include "Game.h"

namespace Intersect
{
	bool RayIntersectBox(const Ray_t& ray, const comp::BoxCollider& boxCollider, float& t);
	bool RayIntersectSphere(const Ray_t& ray, const comp::SphereCollider& sphereCollider, float& t);
}

