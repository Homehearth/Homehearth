#include "EnginePCH.h"
#include "CollisionSystem.h"


void CollisionSystem::AddOnCollisionEnter(Entity entity1, std::function<void(Entity, Entity)> func)
{
	if (m_onCollisionEnter.find(entity1) == m_onCollisionEnter.end())
	{
		m_onCollisionEnter.insert(std::make_pair(entity1, func));
	}
}

void CollisionSystem::OnCollisionEnter(Entity entity1, Entity entity2)
{
	for (int i = 0; i < 2; i++)
	{
		if (m_onCollisionEnter.find(entity1) != m_onCollisionEnter.end())
		{
			if (!entity1.IsNull())
			{
				if (!entity2.IsNull())
				{
					m_onCollisionEnter.at(entity1)(entity1, entity2);
				}
			}
			else
			{
				m_onCollisionEnter.erase(entity1);
			}
		}
		std::swap(entity1, entity2);
	}

}


void CollisionSystem::AddOnCollision(Entity entity1, std::function<void(Entity, Entity)> func)
{
	if (m_onCollision.find(entity1) == m_onCollision.end())
	{
		m_onCollision.insert(std::make_pair(entity1, func));
	}
}

void CollisionSystem::OnCollision(Entity entity1, Entity entity2)
{
	for (int i = 0; i < 2; i++)
	{
		if (m_onCollision.find(entity1) != m_onCollision.end())
		{
			if (!entity1.IsNull())
			{
				if (!entity2.IsNull())
				{
					m_onCollision.at(entity1)(entity1, entity2);
				}
			}
			else
			{
				m_onCollision.erase(entity1);
			}
		}
		std::swap(entity1, entity2);
	}
}

void CollisionSystem::AddOnCollisionExit(Entity entity1, std::function<void(Entity, Entity)> func)
{
	if (m_onCollisionExit.find(entity1) == m_onCollisionExit.end())
	{
		m_onCollisionExit.insert(std::make_pair(entity1, func));
	}
}

void CollisionSystem::OnCollisionExit(Entity entity1, Entity entity2)
{
	for (int i = 0; i < 2; i++)
	{
		if (m_onCollisionExit.find(entity1) != m_onCollisionExit.end())
		{
			if (!entity1.IsNull())
			{
				if (!entity2.IsNull())
				{
					m_onCollisionExit.at(entity1)(entity1, entity2);
				}
			}
			else
			{
				m_onCollisionExit.erase(entity1);
			}
		}
		std::swap(entity1, entity2);
	}
}

bool CollisionSystem::AddPair(Entity entity1, Entity entity2)
{
	size_t collisionSize = m_collisionPairs.size();
	std::pair<Entity, Entity> entityPair = MakeEntityPair(entity1, entity2);
	m_collisionPairs.insert(entityPair);
	if (collisionSize < m_collisionPairs.size())
	{
		return true;
	}
	return false;
}

bool CollisionSystem::RemovePair(Entity entity1, Entity entity2)
{
	std::pair<Entity, Entity> entityPair = MakeEntityPair(entity1, entity2);
	size_t collisionSize = m_collisionPairs.size();
	m_collisionPairs.erase(entityPair);
	if (collisionSize > m_collisionPairs.size())
	{
		return true;
	}
	return false;
}

uint32_t CollisionSystem::GetCollisionCount(Entity entity) const
{
	uint32_t count = 0;
	for (const auto& pair : m_collisionPairs)
	{
		if (entity == pair.first || entity == pair.second)
		{
			count++;
		}
	}
	return count;
}

//CollisionSystem::Projection_t CollisionSystem::GetProjection(sm::Vector3 axis, sm::Vector3* corners)
//{
//	float min = axis.Dot(corners[0]);
//	float max = min;
//
//	for (int i = 1; i < 8; i++)
//	{
//		float p = axis.Dot(corners[i]);
//
//		if (p < min)
//		{
//			min = p;
//		}
//		else if (p > max)
//		{
//			max = p;
//		}
//
//	}
//
//	//Projection_t projection = {min, max};
//	return { min, max };
//}

std::pair<Entity, Entity> CollisionSystem::MakeEntityPair(Entity entity1, Entity entity2)
{
	if (entity1 > entity2)
		return { entity2, entity1 };
	return { entity1, entity2 };
}

//Generates a response when two objects collide with each other,
//Will move both objects if they are both dynamic in different directions depending on how much of
//each corners crosses each other and take the axis that differs the least.
CollisionInfo_t CollisionSystem::Intersection(Entity entity1, Entity entity2)
{
	tag_bits p1Tags = entity1.GetTags();
	tag_bits p2Tags = entity2.GetTags();

	// Sphere - Sphere
	if (p1Tags & TagType::DYNAMIC && p2Tags & TagType::DYNAMIC)
	{
		comp::SphereCollider* p1BoS = entity1.GetComponent<comp::SphereCollider>();
		comp::SphereCollider* p2BoS = entity2.GetComponent<comp::SphereCollider>();
		
		sm::Vector3 vec = sm::Vector3(p2BoS->Center) - sm::Vector3(p1BoS->Center);
		float distance = vec.Length();
		vec.Normalize();
		float radius = (p1BoS->Radius + p2BoS->Radius);
		if (distance < radius)
		{
			float overlap = (radius - distance) * 0.5f;

			return { true, overlap, vec };
		}
	}
	//Sphere - OBB
	else if (p1Tags & TagType::DYNAMIC && p2Tags & TagType::STATIC)
	{
		if (p2Tags & TagType::MAP_BOUNDS && !entity1.GetComponent<comp::Player>())
		{
			return { false, 0.0f, sm::Vector3::Zero };
		}
		comp::SphereCollider* p1BoS = entity1.GetComponent<comp::SphereCollider>();
		comp::OrientedBoxCollider* p2OBB = entity2.GetComponent<comp::OrientedBoxCollider>();

		if (!p1BoS || !p2OBB)
		{
			return { false, 0.f, sm::Vector3::Zero };
		}

		sm::Matrix Translation = sm::Matrix::CreateTranslation(p2OBB->Center).Invert();
		sm::Matrix Rotation = sm::Matrix::CreateFromQuaternion(p2OBB->Orientation).Transpose();
		sm::Matrix obbInverse = Translation * Rotation;
		sm::Vector3 sCenter = p1BoS->Center;
		// Put the sphere in the obb's local space
		sCenter = sm::Vector3::Transform(sCenter, obbInverse);
		sCenter.y = 0.f;

		float minX = (-p2OBB->Extents.x);
		float maxX = p2OBB->Extents.x;
		float minZ = (-p2OBB->Extents.z);
		float maxZ = p2OBB->Extents.z;

		// Get the closest point on the OBB that is inside the sphere
		float closestX = max(minX, min(sCenter.x, maxX));
		float closestZ = max(minZ, min(sCenter.z, maxZ));
		sm::Vector3 ClosestPoint = { closestX, 0.f, closestZ };
		sm::Vector3 pointToSphere = sCenter - ClosestPoint;

		float distance = pointToSphere.Length();

		// If the distance between the point that is inside the Sphere (on the OBB) 
		// is less that the radius we are intersecting
		if (distance < p1BoS->Radius)
		{
			// Center point of the sphere is inside the OBB == EDGE CASE
			if (distance < 0.001f)
			{
				// DeltaLeft, DeltaRight, DeltaTop, DeltaBottom
				float dl = abs(sCenter.x - minX);
				float dr = abs(sCenter.x - maxX);
				float dt = abs(sCenter.z - minZ);
				float db = abs(sCenter.z - maxZ);

				float clampX = sCenter.x;
				float clampZ = sCenter.z;

				// Clamp to the nearest face of the OBB
				if (dt < db && dt < dl && dt < dr)
				{
					clampZ = minZ;
				}
				else if (db < dl && db < dr)
				{
					clampZ = maxZ;
				}
				else if (dl < dr)
				{
					clampX = minX;
				}
				else
				{
					clampX = maxX;
				}
			
				sm::Vector3 closestPoint = { clampX, 0.f, clampZ };

				// This goes inwards to the obb make sure to flip its direction
				sm::Vector3 pointToSphere = (sCenter - closestPoint) * -1.f;
				float overlap = pointToSphere.Length() + p1BoS->Radius;

				pointToSphere = sm::Vector3::TransformNormal(pointToSphere, obbInverse.Invert());
				pointToSphere.Normalize();

				return { true, overlap, pointToSphere };
			}
			else
			{
				float overlap = (p1BoS->Radius - distance);

				pointToSphere.Normalize();
				// To use the normal in world space, transform it back.
				pointToSphere = sm::Vector3::TransformNormal(pointToSphere, obbInverse.Invert());

				return { true, overlap, pointToSphere };
			}
		}
	}

	return { false, 0.0f, sm::Vector3::Zero };




	/*
		DONT DELETE IF WE NEED TO REUSE THIS LATER
	*/

	/*
		comp::BoundingOrientedBox* p1Obb = entity1.GetComponent<comp::BoundingOrientedBox>();
		comp::BoundingOrientedBox* p2Obb = entity2.GetComponent<comp::BoundingOrientedBox>();
		if (p1Obb == nullptr || p2Obb == nullptr)
		{
			//LOG_ERROR("Attempt to perform collision response with or against an entity that does not have a collider component");
			return { false, 0.0f, sm::Vector3::Zero };
		}

		sm::Vector3 p2Corners[8];
		sm::Vector3 p1Corners[8];

		p2Obb->GetCorners(p2Corners);
		p1Obb->GetCorners(p1Corners);

		sm::Vector3 normals[6] = { sm::Vector3(1.0,0.0,0.0f),sm::Vector3(0.0,1.0,0.0f), sm::Vector3(0.0,0.0,1.0f), sm::Vector3(-1.0,0.0,0.0f),sm::Vector3(0.0,-1.0,0.0f), sm::Vector3(0.0,0.0,-1.0f) };
		std::vector<sm::Vector3> allAxis;

		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[0], DirectX::XMLoadFloat4(&p1Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[1], DirectX::XMLoadFloat4(&p1Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[2], DirectX::XMLoadFloat4(&p1Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[3], DirectX::XMLoadFloat4(&p1Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[4], DirectX::XMLoadFloat4(&p1Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[5], DirectX::XMLoadFloat4(&p1Obb->Orientation)));

		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[0], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[1], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[2], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[3], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[4], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
		allAxis.emplace_back(DirectX::XMVector3Rotate(normals[5], DirectX::XMLoadFloat4(&p2Obb->Orientation)));

		float overlap = FLT_MAX;
		sm::Vector3 smallest = sm::Vector3::Zero;

		for (int i = 0; i < allAxis.size(); i++)
		{
			sm::Vector3 axis = allAxis.at(i);

			//Project both shapes on the axis
			Projection_t p1 = GetProjection(axis, p1Corners);
			Projection_t p2 = GetProjection(axis, p2Corners);

			// 1D
			if ((p1.max < p2.min))
			{
				//NO OVERLAP
				return { false, 0.0f, sm::Vector3::Zero };
			}
			else
			{
				//Get the overlap
				float o = p1.max - p2.min;

				if (o < overlap)
				{
					overlap = o;
					smallest = axis;
				}
			}
		}

		return { true,overlap,smallest };
	*/
}

void CollisionSystem::CollisionResponse(CollisionInfo_t collisionInfo, Entity entity1, Entity entity2)
{
	tag_bits p1Tags = entity1.GetTags();
	tag_bits p2Tags = entity2.GetTags();
	sm::Vector3 dir = collisionInfo.smallestVec * collisionInfo.overlap;

	if (p1Tags & TagType::DYNAMIC && p2Tags & TagType::STATIC)
	{
		comp::Transform* transform = entity1.GetComponent<comp::Transform>();
		comp::SphereCollider* BoS = entity1.GetComponent<comp::SphereCollider>();
		if (transform)
		{
			transform->position.x = transform->position.x + dir.x;
			transform->position.z = transform->position.z + dir.z;

			if (BoS)
			{
				BoS->Center = transform->position;
			}

			// make sure client gets updated position
			entity1.UpdateNetwork();
		}
	}
	if (p1Tags & TagType::DYNAMIC && p2Tags & TagType::DYNAMIC)
	{
		comp::Transform* transform1 = entity1.GetComponent<comp::Transform>();
		comp::Transform* transform2 = entity2.GetComponent<comp::Transform>();
		comp::SphereCollider* BoS1 = entity1.GetComponent<comp::SphereCollider>();
		comp::SphereCollider* BoS2 = entity2.GetComponent<comp::SphereCollider>();
		//Dynamic
		if (transform1)
		{
			transform1->position.x = transform1->position.x + dir.x * -1.f;
			transform1->position.z = transform1->position.z + dir.z * -1.f;

			if (BoS1)
			{
				BoS1->Center = transform1->position;
			}

			entity1.UpdateNetwork();
		}
		if (transform2)
		{
			transform2->position.x = transform2->position.x + dir.x;
			transform2->position.z = transform2->position.z + dir.z;

			if (BoS2)
			{
				BoS2->Center = transform2->position;
			}

			entity2.UpdateNetwork();
		}
	}
}