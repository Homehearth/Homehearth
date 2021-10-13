#pragma once
class CollisionSystem
{
private:
    std::set<std::pair<entt::entity, entt::entity>> m_CollisionPairs;
	CollisionSystem() = default;

public:
	~CollisionSystem() = default;
	static auto& Get()
	{
		static CollisionSystem s_instance;
		return s_instance;
	}
	const std::set<std::pair<entt::entity, entt::entity>>& GetCollisions() const;
	void AddPair(const entt::entity e1, const entt::entity e2);
	void RemovePair(const entt::entity e1, const entt::entity e2);
	bool IsColliding(const entt::entity e1, const entt::entity e2);
};

