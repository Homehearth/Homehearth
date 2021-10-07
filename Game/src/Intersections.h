#pragma once
#include "Game.h"

namespace Intersect
{
	const bool RayIntersectBox(const Ray_t& ray, const comp::BoundingOrientedBox& boxCollider, float& t);
	const bool RayIntersectSphere(const Ray_t& ray, const comp::BoundingSphere& sphereCollider, float& t);
}

