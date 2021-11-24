#pragma once
#include "Scene.h"
#include "CollisionSystem.h"
#include "PathFinderManager.h"
#include "CombatSystem.h"
#include "QuadTree.h"

namespace Systems
{
	// abilities
	void UpdateAbilities(HeadlessScene& scene, float dt);
	void CombatSystem(HeadlessScene& scene, float dt);
	void HealingSystem(HeadlessScene& scene, float dt);
	void HeroLeapSystem(HeadlessScene& scene, float dt);

	void HealthSystem(HeadlessScene& scene, float dt, uint32_t& money_ref);
	void SelfDestructSystem(HeadlessScene& scene, float dt);

	void MovementSystem(HeadlessScene& scene, float dt);
	void MovementColliderSystem(HeadlessScene& scene, float dt);
	void LightSystem(Scene& scene, float dt);

	void TransformAnimationSystem(HeadlessScene& scene, float dt);

	void FetchCollidingList(HeadlessScene& scene, QuadTree* qt);
	void ClearCollidingList(HeadlessScene& scene);

	template<typename Collider1, typename Collider2>
	void CheckCollisions(HeadlessScene& scene, float dt);

	void AISystem(HeadlessScene& scene, PathFinderManager* aiHandler);
}

template<typename Collider1, typename Collider2>
inline void Systems::CheckCollisions(HeadlessScene& scene, float dt)
{
	// check objects by using QuadTree
	if (std::is_same<Collider2, dx::BoundingOrientedBox>::value)
	{
		scene.ForEachComponent<comp::ColliderList>([](Entity& e, comp::ColliderList& cl) 
			{
				for (size_t i = 0; i < cl.list.size(); i++)
				{
					CollisionInfo_t collisionInfo = CollisionSystem::Get().Intersection(e, cl.list[i]);

					if (collisionInfo.hasCollided)
					{
						CollisionSystem::Get().CollisionResponse(collisionInfo, e, cl.list[i]);
					}
				}
			});
	}
	else
	{
		auto view1 = scene.GetRegistry()->view<Collider1>();
		auto view2 = scene.GetRegistry()->view<Collider2>();
		for (auto& entity1 = view1.begin(), end = view1.end(); entity1 != end; ++entity1)
		{
			Collider1 collider1 = scene.GetRegistry()->get<Collider1>(*entity1);

			auto [begin, end2] = [&]()
			{
				if constexpr (std::is_same_v<Collider1, Collider2>)
				{
					return std::make_pair(entity1, view1.end());
				}
				else
				{
					return std::make_pair(view2.begin(), view2.end());
				}
			}();

			for (auto& entity2 = begin; entity2 != end2; ++entity2)
			{
				Collider2 collider2 = scene.GetRegistry()->get<Collider2>(*entity2);

				if (*entity1 != *entity2)
				{
					Entity e1(*scene.GetRegistry(), *entity1);
					Entity e2(*scene.GetRegistry(), *entity2);

					if (e1.GetComponent<comp::Tag<DYNAMIC>>() || e2.GetComponent<comp::Tag<DYNAMIC>>())
					{
						CollisionInfo_t collisionInfo = CollisionSystem::Get().Intersection(e1, e2);
						if (collisionInfo.hasCollided)
						{

							if (CollisionSystem::Get().AddPair(e1, e2))
								CollisionSystem::Get().OnCollisionEnter(e1, e2);

							CollisionSystem::Get().OnCollision(e1, e2);

							if (!e1.HasComponent<comp::Tag<TagType::NO_RESPONSE>>() && !e2.HasComponent<comp::Tag<TagType::NO_RESPONSE>>())
								CollisionSystem::Get().CollisionResponse(collisionInfo, e1, e2);
						}
						else
						{
							if (CollisionSystem::Get().RemovePair(e1, e2))
								CollisionSystem::Get().OnCollisionExit(e1, e2);
						}

					}
				}
			}
		}
	}
}
