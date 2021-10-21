#pragma once
class CollisionSystem
{
private:
    std::set<std::pair<Entity, Entity>> m_CollisionPairs;
	std::unordered_map<Entity, int> m_CollisionCount;
	std::unordered_map<Entity, std::function<void(Entity)>> m_OnCollision;
	CollisionSystem() = default;

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
	
	bool IsColliding(const Entity e1, const Entity e2);
	void AddOnCollision(Entity entity1, std::function<void(Entity)> func);
	void OnCollision(Entity entity1, Entity entity2);
};

