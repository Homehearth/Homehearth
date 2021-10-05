#pragma once
#include "Game.h"

namespace Intersect
{
	bool RayIntersectBox(const Ray_t& mouseRay, comp::BoxCollider& boxCollider, float& t);
	bool RayIntersectSphere(const Ray_t& mouseRay, comp::SphereCollider& sphereCollider, float& t);
}

