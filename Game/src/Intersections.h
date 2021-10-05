#pragma once
#include "Game.h"

namespace Intersect
{
	bool RayIntersectBox(const Ray_t& ray, comp::BoxCollider& boxCollider, float& t);
	bool RayIntersectSphere(const Ray_t& ray, comp::SphereCollider& sphereCollider, float& t);
}

