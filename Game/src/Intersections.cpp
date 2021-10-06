#include "Intersections.h"

//Check if the ray intersects with a box collider.
bool Intersect::RayIntersectBox(const Ray_t& ray, const comp::BoxCollider& boxCollider, float& t)
{
	/**
	 * computing all t-values for the ray
	 * and all planes belonging to the faces of the OBB.
	 * It returns the closest positive t-value
	 *
	 */
	const sm::Vector3 rayOrigin = ray.rayPos;
	const sm::Vector3 rayDir = ray.rayDir;

	float tmin = (std::numeric_limits<float>::min)();
	float tmax = (std::numeric_limits<float>::max)();

	sm::Vector3 p = boxCollider.center - rayOrigin;
	for (size_t i = 0; i < 3; i++)
	{
		const float e = boxCollider.norm[i].x * p.x + boxCollider.norm[i].y * p.y + boxCollider.norm[i].z * p.z;
		const float f = boxCollider.norm[i].x * rayDir.x + boxCollider.norm[i].y * rayDir.y + boxCollider.norm[i].z * rayDir.z;

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

bool Intersect::RayIntersectSphere(const Ray_t& ray, const comp::SphereCollider& sphereCollider, float& t)
{
	const sm::Vector3 rayToCenter = ray.rayPos - sphereCollider.center;
	
	//Parameterization to use for the quadtratic formula.
	const float scalarRayDir =  ray.rayDir.Dot(ray.rayDir);
	const float b = 2.0f * rayToCenter.Dot(ray.rayDir);
	const float c = rayToCenter.Dot(rayToCenter) - sphereCollider.radius * sphereCollider.radius;

	const float discriminant = b * b - 4.0f * scalarRayDir * c;
	
	//discriminant < 0 the ray does not intersect sphere.
	//discriminant = 0 the ray touches the sphere in one point
	//discriminant > 0 the ray touches the sphere in two points
	if (discriminant < 0.0f) {
		return false;
	}
	else
	{
		const float numerator = -b + sqrt(discriminant);
		
		if(numerator > 0.0f)
		{
			t = numerator / (2.0f * scalarRayDir);
			return true;
		}
		else
		{
			return false;
		}
	}
}
