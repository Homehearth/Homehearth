#include "EnginePCH.h"
#include "CollisionSystem.h"
const std::set<std::pair<entt::entity, entt::entity>>& CollisionSystem::GetCollisions() const
{
	return m_CollisionPairs;
}

void CollisionSystem::AddPair(const entt::entity e1, const entt::entity e2)
{
	if(e1 > e2)
		m_CollisionPairs.insert({ e1,e2 });
	else
		m_CollisionPairs.insert({ e2,e1 });
}

void CollisionSystem::RemovePair(const entt::entity e1, const entt::entity e2)
{
	if(e1 > e2)
		m_CollisionPairs.erase(std::make_pair(e1, e2));
	else
		m_CollisionPairs.erase(std::make_pair(e2, e1));
}

bool CollisionSystem::IsColliding(const entt::entity e1, const entt::entity e2)
{
	auto it = m_CollisionPairs.find({ e1,e2 });
	if(e2 > e1)
		it = m_CollisionPairs.find({e2,e1});

	if (it == m_CollisionPairs.end())
		return false;
	
	return true;
}
