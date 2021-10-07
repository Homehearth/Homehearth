#pragma once

namespace Intersect
{
	const bool RayIntersectBox(const Ray_t& ray, const comp::BoxCollider& boxCollider, float& t);
	const bool RayIntersectSphere(const Ray_t& ray, const comp::SphereCollider& sphereCollider, float& t);
}
