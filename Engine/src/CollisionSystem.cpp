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

bool CollisionSystem::IsColliding(const Entity e1, const Entity e2)
{
	auto it = m_CollisionPairs.find({ e1,e2 });
	if (e2 > e1)
		it = m_CollisionPairs.find({ e2,e1 });

	if (it == m_CollisionPairs.end())
		return false;

	return true;
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
