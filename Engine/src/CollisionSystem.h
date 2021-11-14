#pragma once
#include "Tags.h"

struct CollisionInfo_t
{
	bool hasCollided;
	float overlap;
	sm::Vector3 smallestVec;
};


class CollisionSystem
{
private:
	std::unordered_map<Entity, int> m_CollisionCount;
	std::unordered_map<Entity, std::function<void(Entity, Entity)>> m_OnCollision;
	CollisionSystem() = default;

	
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
	
	int GetCollisionCounts(Entity entity) const;
	
	void AddOnCollision(Entity entity1, std::function<void(Entity, Entity)> func);
	void OnCollision(Entity entity1, Entity entity2);
	Projection_t GetProjection(sm::Vector3 axis, sm::Vector3* corners);


};

