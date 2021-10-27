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

			if(entity1.GetComponent<comp::Tag<DYNAMIC>>())
				CollisionSystem::Get().CollisionResponse(entity1, entity2);
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
			
			if (entity2.GetComponent<comp::Tag<DYNAMIC>>())
				CollisionSystem::Get().CollisionResponse(entity2, entity1);
		}
		else
		{
			m_OnCollision.erase(entity2);
		}
	}
}


//Generates a response when two objects collide with each other,
//Will move both objects if they are both dynamic in different directions depending on how much of
//each corners crosses each other and take the axis that differs the least.
void CollisionSystem::CollisionResponse(Entity entity1, Entity entity2) const
{
	comp::BoundingOrientedBox* p1Obb = entity1.GetComponent<comp::BoundingOrientedBox>();
	comp::BoundingOrientedBox* p2Obb = entity2.GetComponent<comp::BoundingOrientedBox>();
	
	comp::Transform* p1transform = entity1.GetComponent<comp::Transform>();
	comp::Transform* p2transform = entity2.GetComponent<comp::Transform>();

	if (p1Obb == nullptr || p2Obb == nullptr)
	{
		LOG_ERROR("Attempt to perform collision response with or against an entity that does not have a collider component");
		return;
	}

	sm::Vector3 p2Corners[8];
	sm::Vector3 p1Corners[8];
	p2Obb->GetCorners(p2Corners);
	p1Obb->GetCorners(p1Corners);

	//normal axis for both colliders
	std::vector<sm::Vector3> normalAxis;

	//Get the 3 AXIS from box1 needed to do the projection on
	normalAxis.push_back((p1Corners[1] - p1Corners[0]));
	normalAxis[0].Normalize();
	normalAxis.push_back((p1Corners[2] - p1Corners[1]));
	normalAxis[1].Normalize();
	normalAxis.push_back((p1Corners[0] - p1Corners[4]));
	normalAxis[2].Normalize();

	//Get the 3 AXIS from box2 needed to do the projection on
	normalAxis.push_back((p2Corners[1] - p2Corners[0]));
	normalAxis[3].Normalize();
	normalAxis.push_back((p2Corners[2] - p2Corners[1]));
	normalAxis[4].Normalize();
	normalAxis.push_back((p2Corners[0] - p2Corners[4]));
	normalAxis[5].Normalize();

	std::vector<sm::Vector3> p1Vectors;
	std::vector<sm::Vector3> p2Vectors;

	//Save all the corners
	for (int i = 0; i < 8; i++)
	{
		p1Vectors.emplace_back(p1Corners[i]);
		p2Vectors.emplace_back(p2Corners[i]);
	}

	//Get min-max for the axis for the box
	std::vector<MinMaxProj_t> minMaxProj;
	for(int i = 0; i < normalAxis.size(); i++)
	{
		minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p1Vectors, normalAxis.at(i)));
		minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p2Vectors, normalAxis.at(i)));
	}

	float depth = FLT_MAX;
	sm::Vector3 smallestVec(FLT_MAX, FLT_MAX, FLT_MAX);
	bool isValueSet = false;

	//Find the smallest gap between the min and max corners.
	for (int i = 0; i < static_cast<int>(minMaxProj.size()) - 1; i += 2)
	{
		sm::Vector3 gap;
		//Take projections in the right order depending on entitys position in the world space
		if (minMaxProj[i].maxProj < minMaxProj[i + 1].maxProj)
		{
			gap = p2Vectors[minMaxProj[i + 1].minInxed] - p1Vectors[minMaxProj[i].maxIndex];
		}
		else if (minMaxProj[i].maxProj > minMaxProj[i + 1].maxProj)
		{
			gap = p1Vectors[minMaxProj[i].minInxed] - p2Vectors[minMaxProj[i + 1].maxIndex];
		}
		else
		{
			continue; // Skips if 0.0f
		}

		//Find the axis with lowest difference
		if (abs(gap.x) < abs(depth) && abs(gap.x) > 0.0001f)
		{
			depth = gap.x;
			smallestVec = sm::Vector3(gap.x, 0.0f, 0.0f);
			isValueSet = true;
		}
		if (abs(gap.y) < abs(depth) && abs(gap.y) > 0.0001f)
		{
			depth = gap.y;
			smallestVec = sm::Vector3(0.0f, gap.y, 0.0f);
			isValueSet = true;
		}
		if (abs(gap.z) < abs(depth) && abs(gap.z) > 0.0001f)
		{
			depth = gap.z;
			smallestVec = sm::Vector3(0.0f, 0.0f, gap.z);
			isValueSet = true;
		}

	}

	//Reset to 0.001 (happens if boxes share exactly the same corners pos)
	if (!isValueSet)
	{
		smallestVec = sm::Vector3(0.01f, 0.01f, 0.01f);
	}

	//Move the entity away from the other entity
	sm::Vector3 moveVec;
	
	if(entity2.GetComponent<comp::Tag<STATIC>>() != nullptr)
		moveVec = ((smallestVec));
	else
		moveVec = ((smallestVec));
	
	moveVec *= 1.1f;
	if ((p2transform->position - (p1transform->position + moveVec)).Length() > (p2transform->position -
		p1transform->position).Length())
	{
		p1transform->position += (moveVec);
		p1Obb->Center = p1transform->position;
	}
	else
	{
		p1transform->position += (moveVec * -1.0f);
		p1Obb->Center = p1transform->position;
	}

	//If still colliding should make another iteration through the function to find the other axis
	if(entity2.GetComponent<comp::Tag<STATIC>>() && p1Obb->Intersects(*p2Obb))
	{
		this->CollisionResponse(entity1, entity2);
	}
}




MinMaxProj_t CollisionSystem::GetMinMax(std::vector<sm::Vector3> boxVectors, sm::Vector3 boxAxis)
{
	MinMaxProj_t minMaxProj;

	minMaxProj.minProj = boxVectors.at(0).Dot(boxAxis);
	minMaxProj.maxProj = boxVectors.at(0).Dot(boxAxis);
	minMaxProj.minInxed = 0;
	minMaxProj.maxIndex = 0;
	minMaxProj.axisProjectOn = boxAxis;
	
	for(int i = 1; i < boxVectors.size(); i++)
	{
		float currentProj = boxVectors[i].Dot(boxAxis);
		//select the maximum projection on axis to corresponding box corners
		if(minMaxProj.minProj > currentProj)
		{
			minMaxProj.minProj = currentProj;
			minMaxProj.minInxed = i;
		}
		//select the minimum projection on axis to corresponding box corners
		if(currentProj > minMaxProj.maxProj)
		{
			minMaxProj.maxProj = currentProj;
			minMaxProj.maxIndex = i;
		}
	}

	return minMaxProj;		
}
