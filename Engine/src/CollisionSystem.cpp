#include "EnginePCH.h"
#include "CollisionSystem.h"



//Returns number of how many colliders the entity is colliding with
const int CollisionSystem::getCollisionCounts(Entity entity) const
{
	int count = 0;
	if (m_CollisionCount.find(entity) != m_CollisionCount.end())
	{
		count = this->m_CollisionCount.at(entity);
	}

	return count;
}




const std::set<std::pair<Entity, Entity>>& CollisionSystem::GetCollisions() const
{
	return m_CollisionPairs;
}




void CollisionSystem::AddPair(const Entity e1, const Entity e2)
{
#ifdef _DEBUG
	//Increase collision count for each Entity
	if (m_CollisionPairs.find(std::make_pair(e1, e2)) == m_CollisionPairs.end())
	{
		if (m_CollisionCount.find(e1) == m_CollisionCount.end())
		{
			m_CollisionCount.insert(std::make_pair(e1, 1));
		}
		else
		{
			m_CollisionCount.at(e1)++;
		}

		if (m_CollisionCount.find(e2) == m_CollisionCount.end())
		{
			m_CollisionCount.insert(std::make_pair(e2, 1));
		}
		else
		{
			m_CollisionCount.at(e2)++;
		}
	}
#endif

	if (e1 > e2)
		m_CollisionPairs.insert({ e1,e2 });
	else
		m_CollisionPairs.insert({ e2,e1 });

}




void CollisionSystem::RemovePair(const Entity e1, const Entity e2)
{

#ifdef _DEBUG
	//Decrease collision count for each Entity
	if (m_CollisionPairs.find(std::make_pair(e1, e2)) != m_CollisionPairs.end() || m_CollisionPairs.find(std::make_pair(e2, e1)) != m_CollisionPairs.end())
	{
		if (m_CollisionCount.find(e1) != m_CollisionCount.end() && m_CollisionCount.find(e1)->second > 0)
		{
			m_CollisionCount.at(e1)--;
		}

		if (m_CollisionCount.find(e2) != m_CollisionCount.end() && m_CollisionCount.find(e2)->second > 0)
		{
			m_CollisionCount.at(e2)--;
		}
	}
#endif


	if (e1 > e2)
		m_CollisionPairs.erase(std::make_pair(e1, e2));
	else
		m_CollisionPairs.erase(std::make_pair(e2, e1));


}




void CollisionSystem::AddOnCollision(Entity entity1, std::function<void(Entity)> func)
{
	if (m_OnCollision.find(entity1) == m_OnCollision.end())
	{
		m_OnCollision.insert(std::make_pair(entity1, func));
	}
}

void CollisionSystem::OnCollision(Entity entity1, Entity entity2)
{
	if (m_OnCollision.find(entity1) != m_OnCollision.end())
	{
		if (!entity1.IsNull())
		{
			m_OnCollision.at(entity1)(entity2);
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
			m_OnCollision.at(entity2)(entity1);
		}
		else
		{
			m_OnCollision.erase(entity2);
		}
	}
}


CollisionSystem::Projection_t CollisionSystem::GetProjection(sm::Vector3 axis, sm::Vector3* corners)
{
	double min = axis.Dot(corners[0]);
	double max = min;

	for(int i = 1; i < 8; i++)
	{
		double p = axis.Dot(corners[i]);

		if(p < min)
		{
			min = p;
		}
		else if(p > max)
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
	comp::BoundingOrientedBox* p1Obb = entity1.GetComponent<comp::BoundingOrientedBox>();
	comp::BoundingOrientedBox* p2Obb = entity2.GetComponent<comp::BoundingOrientedBox>();

	if (p1Obb == nullptr || p2Obb == nullptr)
	{
		LOG_ERROR("Attempt to perform collision response with or against an entity that does not have a collider component");
		return {false, 0.0, sm::Vector3::Zero};
	}
	
	sm::Vector3 p2Corners[8];
	sm::Vector3 p1Corners[8];
	
	p2Obb->GetCorners(p2Corners);
	p1Obb->GetCorners(p1Corners);
	
	sm::Vector3 normals[6] = { sm::Vector3(1.0,0.0,0.0f),sm::Vector3(0.0,1.0,0.0f), sm::Vector3(0.0,0.0,1.0f), sm::Vector3(-1.0,0.0,0.0f),sm::Vector3(0.0,-1.0,0.0f), sm::Vector3(0.0,0.0,-1.0f) };
	std::vector<sm::Vector3> allAxis;

	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[0], DirectX::XMLoadFloat4(& p1Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[1], DirectX::XMLoadFloat4(& p1Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[2], DirectX::XMLoadFloat4(& p1Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[3], DirectX::XMLoadFloat4(& p1Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[4], DirectX::XMLoadFloat4(& p1Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[5], DirectX::XMLoadFloat4(& p1Obb->Orientation)));
	
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[0], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[1], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[2], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[3], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[4], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
	allAxis.emplace_back(DirectX::XMVector3Rotate(normals[5], DirectX::XMLoadFloat4(&p2Obb->Orientation)));
	
	double overlap = DBL_MAX;
	sm::Vector3 smallest = sm::Vector3::Zero;

	for(int i = 0; i < allAxis.size(); i++)
	{
		sm::Vector3 axis = allAxis.at(i);
		
		//Project both shapes on the axis
		Projection_t p1 = GetProjection(axis, p1Corners);
		Projection_t p2 = GetProjection(axis, p2Corners);

		// 1D
		if((p1.max < p2.min))
		{
			//NO OVERLAP
			return { false,0.0, sm::Vector3::Zero };
		}
		else
		{
			//Get the overlap
			double o = p1.max - p2.min;

			if(o < overlap)
			{
				overlap = o;
				smallest = axis;
			}

		}
	}

	return { true,overlap,smallest };
	
}

void CollisionSystem::CollisionResponse(CollisionInfo_t collisionInfo, Entity entity1, Entity entity2)
{
	if(entity1.GetComponent<comp::Tag<DYNAMIC>>() && entity2.GetComponent<comp::Tag<STATIC>>())
	{
		comp::Transform* transform = entity1.GetComponent<comp::Transform>();
		comp::BoundingOrientedBox* obb = entity1.GetComponent<comp::BoundingOrientedBox>();
		if (transform)
		{
			transform->position = transform->position + sm::Vector3(collisionInfo.smallestVec * (float)collisionInfo.overlap * -1.1f);
			
			if (obb)
				obb->Center = transform->position;
		}
			
	}
	else if(entity2.GetComponent<comp::Tag<DYNAMIC>>() && entity1.GetComponent<comp::Tag<STATIC>>())
	{
		comp::Transform* transform = entity2.GetComponent<comp::Transform>();
		comp::BoundingOrientedBox* obb = entity2.GetComponent<comp::BoundingOrientedBox>();
		
		if (transform)
		{
			transform->position = transform->position + sm::Vector3(collisionInfo.smallestVec * (float)collisionInfo.overlap * 1.1f);
			
			if (obb)
				obb->Center = transform->position;
		}
			
	}
	else if(entity2.GetComponent<comp::Tag<DYNAMIC>>() && entity1.GetComponent<comp::Tag<DYNAMIC>>())
	{
		comp::Transform* transform1 = entity1.GetComponent<comp::Transform>();
		comp::Transform* transform2 = entity2.GetComponent<comp::Transform>();
		comp::BoundingOrientedBox* obb1 = entity1.GetComponent<comp::BoundingOrientedBox>();
		comp::BoundingOrientedBox* obb2 = entity2.GetComponent<comp::BoundingOrientedBox>();
		//Dynamic
		if (transform1)
		{
			transform1->position = transform1->position + (sm::Vector3(collisionInfo.smallestVec * (float)collisionInfo.overlap * -1.1f) / 2.0f);

			if (obb1)
				obb1->Center = transform1->position;
		}

		if (transform2)
		{
			transform2->position = transform2->position + (sm::Vector3(collisionInfo.smallestVec * (float)collisionInfo.overlap) / 2.0f * 1.1f);

			if (obb2)
				obb2->Center = transform2->position;
		}
			
	}
}
