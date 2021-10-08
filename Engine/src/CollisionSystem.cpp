#include "EnginePCH.h"
#include "CollisionSystem.h"
const std::unordered_set<std::pair<Entity, Entity>, CollisionSystem::pair_hash>& CollisionSystem::getCollisions() const
{
	//return m_CollisionPairs;
	return m_CollisionPairs;
}

void CollisionSystem::addPair(const Entity e1, const Entity e2)
{
	m_CollisionPairs.insert(e1,e2);
}
