#include "EnginePCH.h"
#include "Intersections.h"

//CheckCollisions if the ray intersects with a box collider.
const bool Intersect::RayIntersectBox(const Ray_t& ray, const comp::BoundingOrientedBox& boxCollider, float& t)
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

	sm::Vector3 p = boxCollider.Center - rayOrigin;

	std::array<sm::Vector3, 3> norms;
	norms[0] = sm::Vector3(1.0f, 0.0f, 0.0f);
	norms[1] = sm::Vector3(0.0f, 1.0f, 0.0f);
	norms[2] = sm::Vector3(0.0f, 0.0f, 1.0f);
	
	norms[0] = sm::Vector3::Transform(norms[0], boxCollider.Orientation);
	norms[1] = sm::Vector3::Transform(norms[1], boxCollider.Orientation);
	norms[2] = sm::Vector3::Transform(norms[2], boxCollider.Orientation);

	float halfSize[3];
	halfSize[0] = boxCollider.Extents.x;
	halfSize[1] = boxCollider.Extents.y;
	halfSize[2] = boxCollider.Extents.z;
	
	for (size_t i = 0; i < 3; i++)
	{
		const float e = norms[i].x * p.x + norms[i].y * p.y + norms[i].z * p.z;
		const float f = norms[i].x * rayDir.x + norms[i].y * rayDir.y + norms[i].z * rayDir.z;

		//CheckCollisions normal face is not ortogonal to ray direction
		if (abs(f) > 0.00001f)
		{
			float t1 = (e + halfSize[i]) / f;
			float t2 = (e - halfSize[i]) / f;

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
		else if (-e - halfSize[i] > 0 || -e + halfSize[i] < 0)
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

const bool Intersect::RayIntersectSphere(const Ray_t& ray, const comp::BoundingSphere& sphereCollider, float& t)
{
	const sm::Vector3 rayToCenter = ray.rayPos - sphereCollider.Center;
	
	//Parameterization to use for the quadtratic formula.
	const float scalarRayDir =  ray.rayDir.Dot(ray.rayDir);
	const float b = 2.0f * rayToCenter.Dot(ray.rayDir);
	const float c = rayToCenter.Dot(rayToCenter) - sphereCollider.Radius * sphereCollider.Radius;

	const float discriminant = b * b - 4.0f * scalarRayDir * c;
	
	//discriminant < 0 the ray does not intersect sphere.
	//discriminant = 0 the ray touches the sphere in one point
	//discriminant > 0 the ray touches the sphere in two points
	if (discriminant < 0.0f)
	{
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
