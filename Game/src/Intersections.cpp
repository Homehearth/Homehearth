#include "Intersections.h"

//Check if the mouse ray intersects with a box collider.
bool Intersect::MouseRayIntersectBox(comp::BoxCollider& boxCollider, float& t)
{
	/**
	 * computing all t-values for the ray
	 * and all planes belonging to the faces of the OBB.
	 * It returns the closest positive t-value
	 *
	 */
	sm::Vector3 rayOrigin = InputSystem::Get().GetMouseRay().rayPos;
	sm::Vector3 rayDir = InputSystem::Get().GetMouseRay().rayDir;

	float tmin = (std::numeric_limits<float>::min)();
	float tmax = (std::numeric_limits<float>::max)();

	sm::Vector3 p = boxCollider.center - rayOrigin;
	for (size_t i = 0; i < 3; i++)
	{
		float e = boxCollider.norm[i].x * p.x + boxCollider.norm[i].y * p.y + boxCollider.norm[i].z * p.z;
		float f = boxCollider.norm[i].x * rayDir.x + boxCollider.norm[i].y * rayDir.y + boxCollider.norm[i].z * rayDir.z;

		//Check normal face is not ortogonal to ray direction
		if (abs(f) > 0.00001f)
		{
			float t1 = (e + boxCollider.halfSize[i]) / f;
			float t2 = (e - boxCollider.halfSize[i]) / f;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}
			if (t1 > tmin)
			{
				tmin = t1;
			}
			if (t2 < tmax)
			{
				tmax = t2;
			}
			if (tmin > tmax)
			{
				return false;
			}
			//if true, then the box is behind the rayorigin.
			if (tmax < 0)
			{
				return false;
			}

		}
		/**executed if the ray is parallel to the slab
		 * (and so cannot intersect it); it tests if the ray is outside the slab.
		 * If so, then the ray misses the box and the test terminates.
		 */
		else if (-e - boxCollider.halfSize[i] > 0 || -e + boxCollider.halfSize[i] < 0)
		{
			return false;
		}

	}
	
	if (tmin > 0)
		t = tmin;
	else
		t = tmax;

	return true;
}

bool Intersect::MouseRayIntersectSphere(comp::SphereCollider& sphereCollider, float& t)
{
	//Todo: Change to Epsilon comparison
	if (sphereCollider.radius == 0.f)
	{
		return false;
	}
	sm::Vector3 rayOrigin = InputSystem::Get().GetMouseRay().rayPos;
	sm::Vector3 rayDir = InputSystem::Get().GetMouseRay().rayDir;

	sm::Vector3 objectPos = sphereCollider.centerOffset;
	sm::Vector3 rayToCenter = objectPos - rayOrigin;

	double scalar = rayToCenter.Dot(rayDir);

	if (scalar < 0 && sphereCollider.radius * sphereCollider.radius < rayToCenter.Dot(rayToCenter))
	{
		return false;
	}
	double m2 = rayToCenter.Dot(rayToCenter) - (scalar * scalar);
	if (m2 > sphereCollider.radius * sphereCollider.radius)
	{
		return false;
	}
	double axis = sqrt(sphereCollider.radius * sphereCollider.radius - m2);
	double intersectionLength = scalar - axis, intersectionLength2 = scalar + axis;
	t = (intersectionLength < intersectionLength2) ? intersectionLength : intersectionLength2;
	return true;
}
