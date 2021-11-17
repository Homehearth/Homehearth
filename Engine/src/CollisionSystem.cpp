#include "EnginePCH.h"
#include "CollisionSystem.h"


void CollisionSystem::AddOnCollisionEnter(Entity entity1, std::function<bool(Entity, Entity)> func)
{
	if (m_onCollisionEnter.find(entity1) == m_onCollisionEnter.end())
	{
		m_onCollisionEnter.insert(std::make_pair(entity1, func));
	}
}

bool CollisionSystem::OnCollisionEnter(Entity entity1, Entity entity2)
{
	bool doResponse = RESPONSE;
	for (int i = 0; i < 2; i++)
	{
		if (m_onCollisionEnter.find(entity1) != m_onCollisionEnter.end())
		{
			if (!entity1.IsNull())
			{
				if (!entity2.IsNull())
				{
					if (!m_onCollisionEnter.at(entity1)(entity1, entity2))
						doResponse = NO_RESPONSE;
				}
			}
			else
			{
				m_onCollisionEnter.erase(entity1);
			}
		}
		std::swap(entity1, entity2);
	}
	return doResponse;
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

CollisionSystem::Projection_t CollisionSystem::GetProjection(sm::Vector3 axis, sm::Vector3* corners)
{
	float min = axis.Dot(corners[0]);
	float max = min;

	for (int i = 1; i < 8; i++)
	{
		float p = axis.Dot(corners[i]);

		if (p < min)
		{
			min = p;
		}
		else if (p > max)
		{
			max = p;
		}

	}

	//Projection_t projection = {min, max};
	return { min, max };
}

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
		comp::BoundingSphere* p1BoS = entity1.GetComponent<comp::BoundingSphere>();
		comp::BoundingSphere* p2BoS = entity2.GetComponent<comp::BoundingSphere>();
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
		comp::BoundingSphere* p1BoS = entity1.GetComponent<comp::BoundingSphere>();
		comp::BoundingOrientedBox* p2OBB = entity2.GetComponent<comp::BoundingOrientedBox>();

		sm::Matrix Translation = sm::Matrix::CreateTranslation(p2OBB->Center).Invert();
		sm::Matrix Rotation = sm::Matrix::CreateFromQuaternion(p2OBB->Orientation).Transpose();
		sm::Matrix obbInverse = Translation * Rotation;
		sm::Vector3 obbLocalPos = p2OBB->Center;
		// Put the obb in local space
		obbLocalPos = sm::Vector3::Transform(obbLocalPos, obbInverse);
		sm::Vector3 sCenter = p1BoS->Center;
		// Put the sphere in the obb's local space
		sCenter = sm::Vector3::Transform(sCenter, obbInverse);

		// Get the closest point on the OBB that is inside the sphere
		float closestX = max(obbLocalPos.x - p2OBB->Extents.x, min(sCenter.x, obbLocalPos.x + p2OBB->Extents.x));
		float closestY = max(obbLocalPos.y - p2OBB->Extents.y, min(sCenter.y, obbLocalPos.y + p2OBB->Extents.y));
		float closestZ = max(obbLocalPos.z - p2OBB->Extents.z, min(sCenter.z, obbLocalPos.z + p2OBB->Extents.z));
		sm::Vector3 ClosestPoint = { closestX, closestY, closestZ };
		sm::Vector3 pointToSphere = sCenter - ClosestPoint;

		float distance = pointToSphere.Length();

		// If the distance between the point that is inside the Sphere (on the OBB) 
		// is less that the radius we are intersecting
		if (distance < p1BoS->Radius)
		{
			// Center point of the sphere is inside the OBB == EDGE CASE
			if (abs(distance) < 0.001f)
			{
				float clampX;
				float clampY;
				float clampZ;
				
				// Gives a point on the nearest face of the OBB
				if (sCenter.x < obbLocalPos.x)
				{
					clampX = obbLocalPos.x - p2OBB->Extents.x;
				}
				else
				{
					clampX = obbLocalPos.x + p2OBB->Extents.x;
				}
				if (sCenter.y < obbLocalPos.y)
				{
					clampY = obbLocalPos.y - p2OBB->Extents.y;
				}
				else
				{
					clampY = obbLocalPos.y + p2OBB->Extents.y;
				}
				if (sCenter.z < obbLocalPos.z)
				{
					clampZ = obbLocalPos.z - p2OBB->Extents.z;
				}
				else
				{
					clampZ = obbLocalPos.z + p2OBB->Extents.z;
				}
				
				sm::Vector3 closestPoint(clampX, clampY, clampZ);
				sm::Vector3 faceToSphere = (sCenter - closestPoint);

				sm::Vector3 normals[4] = { sm::Vector3(1.0f, 0.0f, 0.0f), sm::Vector3(0.0f, 0.0f, 1.0f),
											sm::Vector3(-1.0f, 0.0f, 0.0f), sm::Vector3(0.0f, 0.0f, -1.0f) };

				// Project the vector from the face point to the sphere on the normals.
				// This will give us the direction normal of where to push the sphere towards
				// The length of the projection vector and the sphere radius is the EXACT
				// length that is needed to be outside the OBB.
				float overlap = FLT_MAX;
				int index = 0;
				for (int i = 0; i < 4; i++)
				{
					float dot = faceToSphere.Dot(normals[i]);
					float projectionLength = (dot * normals[i]).Length();

					if (projectionLength < overlap && dot < 0.0f)
					{
						overlap = projectionLength;
						index = i;
					}
				}

				normals[index] = sm::Vector3::TransformNormal(normals[index], obbInverse.Invert());
				normals[index].Normalize();

				return { true, p1BoS->Radius + overlap, normals[index] };
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

	if (p1Tags & TagType::DYNAMIC && p2Tags & TagType::STATIC)
	{
		comp::Transform* transform = entity1.GetComponent<comp::Transform>();
		comp::BoundingSphere* BoS = entity1.GetComponent<comp::BoundingSphere>();
		if (transform)
		{
			transform->position = transform->position + sm::Vector3(collisionInfo.smallestVec * collisionInfo.overlap);
			transform->position.y = 0.f;

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
		comp::BoundingSphere* BoS1 = entity1.GetComponent<comp::BoundingSphere>();
		comp::BoundingSphere* BoS2 = entity2.GetComponent<comp::BoundingSphere>();
		//Dynamic
		if (transform1)
		{
			transform1->position = transform1->position + (sm::Vector3(collisionInfo.smallestVec * collisionInfo.overlap * -1.0f));
			transform1->position.y = 0.f;

			if (BoS1)
			{
				BoS1->Center = transform1->position;
			}

			entity1.UpdateNetwork();
		}
		if (transform2)
		{
			transform2->position = transform2->position + (sm::Vector3(collisionInfo.smallestVec * collisionInfo.overlap));
			transform2->position.y = 0.f;

			if (BoS2)
			{
				BoS2->Center = transform2->position;
			}

			entity2.UpdateNetwork();
		}
	}
}
