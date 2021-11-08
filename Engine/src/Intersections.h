#pragma once

namespace Intersect
{
	const bool RayIntersectBox(const Ray_t& ray, const comp::BoundingOrientedBox& boxCollider, float& t);
	const bool RayIntersectSphere(const Ray_t& ray, const comp::BoundingSphere& sphereCollider, float& t);
	const bool RayIntersectPlane(const Ray_t& ray, const comp::PlaneCollider& planeCollider, float& t);
}
