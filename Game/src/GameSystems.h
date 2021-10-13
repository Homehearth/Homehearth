#pragma once
#include "EnginePCH.h"
#include "Intersections.h"

namespace GameSystems
{
	void UserInputSystem(Scene& scene, Client& client);
	void MRayIntersectBoxSystem(Scene& scene);

	template<typename Collider1, typename Collider2>
	void CheckCollisions(Scene& scene)
	{
			auto view1 = scene.GetRegistry()->view<Collider1>();
			auto view2 = scene.GetRegistry()->view<Collider2>();
			for(auto entity1 = view1.begin(), end = view1.end(); entity1 != end; ++entity1)
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
				
				for(auto entity2 = begin; entity2 != end2; ++entity2)
				{
					Collider2 collider2 = scene.GetRegistry()->get<Collider2>(*entity2);
					
					if (*entity1 != *entity2)
					{
						if (collider1.Intersects(collider2))
						{
							scene.publish<ESceneCollision>(*entity1, *entity2);
						}
						else
						{
							CollisionSystem::Get().RemovePair(*entity1, *entity2);
						}
					}
				}
			}
	}
}
