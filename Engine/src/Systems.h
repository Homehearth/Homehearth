#pragma once
#include "Scene.h"
#include "CollisionSystem.h"

namespace Systems
{
	void CombatSystem(HeadlessScene& scene, float dt);
	void MovementSystem(HeadlessScene& scene, float dt);
	void MovementColliderSystem(HeadlessScene& scene, float dt);
	void LightSystem(Scene& scene, float dt);

	template<typename Collider1, typename Collider2>
	void CheckCollisions(HeadlessScene& scene, float dt);
	void AISystem(HeadlessScene& scene);


}

template<typename Collider1, typename Collider2>
inline void Systems::CheckCollisions(HeadlessScene& scene, float dt)
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

				if(e1.GetComponent<comp::Tag<DYNAMIC>>() || e2.GetComponent<comp::Tag<DYNAMIC>>())
				{
					CollisionInfo_t collisionInfo = CollisionSystem::Get().Intersection(e1, e2);
					if (collisionInfo.hasCollided)
					{
						scene.publish<ESceneCollision>(e1, e2, dt);
						CollisionSystem::Get().CollisionResponse(collisionInfo, e1, e2);
					}
					else
					{
						CollisionSystem::Get().RemovePair(e1, e2);
					}
				}
			}
		}
	}

}
