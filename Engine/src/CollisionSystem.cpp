#include "EnginePCH.h"
#include "CollisionSystem.h"

//Returns number of how many colliders the entity is colliding with
int CollisionSystem::GetCollisionCounts(Entity entity) const
{
	int count = 0;
	if (m_CollisionCount.find(entity) != m_CollisionCount.end())
	{
		count = this->m_CollisionCount.at(entity);
	}

	return count;
}


void CollisionSystem::AddOnCollision(Entity entity1, std::function<bool(Entity, Entity)> func)
{
	if (m_OnCollision.find(entity1) == m_OnCollision.end())
	{
		m_OnCollision.insert(std::make_pair(entity1, func));
	}
}

bool CollisionSystem::OnCollision(Entity entity1, Entity entity2)
{

	bool doResponse = RESPONSE;
	if (m_OnCollision.find(entity1) != m_OnCollision.end())
	{
		if (!entity1.IsNull())
		{
			if (!entity2.IsNull())
			{
				if (!m_OnCollision.at(entity1)(entity1, entity2))
					doResponse = NO_RESPONSE;
			}
		}
		else
		{
			m_OnCollision.erase(entity1);
		}
	}
	if (m_OnCollision.find(entity2) != m_OnCollision.end())
	{
		if (!entity2.IsNull())
		{
			if (!entity1.IsNull())
			{
				if (!m_OnCollision.at(entity1)(entity1, entity2))
					doResponse = NO_RESPONSE;
			}
		}
		else
		{
			m_OnCollision.erase(entity2);
		}
	}

	return doResponse;
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
		else
		{
			return { false, 0.0f, sm::Vector3::Zero };
		}
	}
	//Sphere - OBB
	else if (p1Tags & TagType::DYNAMIC && p2Tags & TagType::STATIC)
	{
		comp::BoundingSphere* p1BoS = entity1.GetComponent<comp::BoundingSphere>();
		comp::BoundingOrientedBox* p2OBB = entity2.GetComponent<comp::BoundingOrientedBox>();

		sm::Matrix Translation = sm::Matrix::CreateTranslation(p2OBB->Center);
		sm::Matrix Rotation = sm::Matrix::CreateFromQuaternion(p2OBB->Orientation);
		sm::Matrix obbInverse = (Translation * Rotation).Invert();
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
		// MIGHT NEED TO ADD MORE LOGIC IF THE SPHERE CENTER EVER COLLIDES WITH THE BOX
		if (distance < p1BoS->Radius)
		{
			float overlap = (p1BoS->Radius - distance);

			pointToSphere.Normalize();
			// To use the normal in world space, transform it back.
			pointToSphere = sm::Vector3::TransformNormal(pointToSphere, obbInverse.Invert());
			// For some stupid reason we get the axis inverted, invert it back.
			pointToSphere * -1.f;

			return { true, overlap, pointToSphere };
		}
		else
		{
			return { false, 0.0f, sm::Vector3::Zero };
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
