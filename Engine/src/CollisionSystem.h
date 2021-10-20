#pragma once
class CollisionSystem
{
private:
    std::set<std::pair<entt::entity, entt::entity>> m_CollisionPairs;
	std::unordered_map<entt::entity, int> m_CollisionCount;
	std::unordered_map<entt::entity, std::function<void(entt::entity)>> m_OnCollision;
	CollisionSystem() = default;

public:
	~CollisionSystem() = default;
	static auto& Get()
	{
		static CollisionSystem s_instance;
		return s_instance;
	}
	const int getCollisionCounts(entt::entity entity) const;
	const std::set<std::pair<entt::entity, entt::entity>>& GetCollisions() const;
	void AddPair(const entt::entity e1, const entt::entity e2);
	void RemovePair(const entt::entity e1, const entt::entity e2);
	
	bool IsColliding(const entt::entity e1, const entt::entity e2);
	void AddOnCollision(entt::entity entity1, std::function<void(entt::entity)> func);
	void OnCollision(entt::entity entity1, entt::entity entity2);
};

