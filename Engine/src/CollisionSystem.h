#pragma once
#include "Tags.h"

#define RESPONSE true
#define NO_RESPONSE false

struct CollisionInfo_t
{
	bool hasCollided;
	float overlap;
	sm::Vector3 smallestVec;
};



namespace std
{
	template <> struct hash<std::pair<Entity, Entity>>
	{
		size_t operator()(const std::pair<Entity, Entity>& other)const
		{
			return std::hash<Entity>()(other.first) ^ std::hash<Entity>()(other.second);
		}
	};
}

class CollisionSystem
{
private:
	std::unordered_map<Entity, std::function<bool(Entity, Entity)>> m_onCollisionEnter;
	std::unordered_map<Entity, std::function<void(Entity, Entity)>> m_onCollision;
	std::unordered_map<Entity, std::function<void(Entity, Entity)>> m_onCollisionExit;

	std::unordered_set<std::pair<Entity, Entity>> m_collisionPairs;

	CollisionSystem() = default;
	
	std::pair<Entity, Entity> MakeEntityPair(Entity entity1, Entity entity2);

public:
	~CollisionSystem() = default;
	static auto& Get()
	{
		static CollisionSystem s_instance;
		return s_instance;
	}

	struct Projection_t
	{
		float min;
		float max;
	};

	CollisionInfo_t Intersection(Entity entity1, Entity entity2);
	void CollisionResponse(CollisionInfo_t collisionInfo, Entity entity1, Entity entity2);
		
	void AddOnCollisionEnter(Entity entity1, std::function<bool(Entity, Entity)> func);
	bool OnCollisionEnter(Entity entity1, Entity entity2);

	void AddOnCollision(Entity entity1, std::function<void(Entity, Entity)> func);
	void OnCollision(Entity entity1, Entity entity2);

	void AddOnCollisionExit(Entity entity1, std::function<void(Entity, Entity)> func);
	void OnCollisionExit(Entity entity1, Entity entity2);

	bool AddPair(Entity entity1, Entity entity2);
	bool RemovePair(Entity entity1, Entity entity2);

	uint32_t GetCollisionCount(Entity entity) const;

	Projection_t GetProjection(sm::Vector3 axis, sm::Vector3* corners);


};

