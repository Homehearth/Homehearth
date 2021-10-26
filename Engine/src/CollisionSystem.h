#pragma once
#include "Tags.h"
class CollisionSystem
{
private:
    std::set<std::pair<Entity, Entity>> m_CollisionPairs;
	std::unordered_map<Entity, int> m_CollisionCount;
	std::unordered_map<Entity, std::function<void(Entity)>> m_OnCollision;
	CollisionSystem() = default;
	
	void CollisionResponsDynamic(Entity entity1, Entity entity2);
	void CollisionResponsStatic(Entity entity1, Entity staticEntity2);
public:
	~CollisionSystem() = default;
	static auto& Get()
	{
		static CollisionSystem s_instance;
		return s_instance;
	}
	const int getCollisionCounts(Entity entity) const;
	const std::set<std::pair<Entity, Entity>>& GetCollisions() const;
	void AddPair(const Entity e1, const Entity e2);
	void RemovePair(const Entity e1, const Entity e2);
	
	void AddOnCollision(Entity entity1, std::function<void(Entity)> func);
	void OnCollision(Entity entity1, Entity entity2, float dt);
	void CollisionRespons(Entity entity1, Entity entity2, float dt);
    MinMaxProj_t GetMinMax(std::vector<sm::Vector3> boxVectors, sm::Vector3 boxAxis);
};

