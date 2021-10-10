#include "GameSystems.h"

using namespace network;
void GameSystems::UserInputSystem(Scene& scene, Client& client)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::Player>([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
		{
			velocity.vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
			velocity.vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
		});
}

//System check if mouse ray intersects any of the box collider components in scene
void GameSystems::MRayIntersectBoxSystem(Scene& scene)
{
	float t = 0;
	
	scene.ForEachComponent<comp::BoundingOrientedBox, comp::Transform>([&](Entity entity, comp::BoundingOrientedBox& boxCollider, comp::Transform& transform)
    {
		//Collided with mouse TODO make it do someting?
		if(Intersect::RayIntersectBox(InputSystem::Get().GetMouseRay(), boxCollider, t))
		{
			LOG_INFO("Mouseray HIT box detected!");
		}
	});
}

//Goes through all the collider in scene and checks if they collides with each other.
void GameSystems::CollisionUpdateSystem(Scene& scene)
{
	auto viewObb = scene.GetRegistry()->view<comp::BoundingOrientedBox>();
	auto viewSphere = scene.GetRegistry()->view<comp::BoundingSphere>();
	auto pack = viewObb; //| viewSphere;
	
	for (auto entity1 = pack.begin(), end = pack.end(); entity1 != end; ++entity1)
	{
		comp::BoundingOrientedBox obb1 = pack.get<comp::BoundingOrientedBox>(*entity1);
		//comp::BoundingSphere sphere1 = pack.get<comp::BoundingSphere>(*entity1);
		
		for (auto entity2 = entity1; entity2 != end; ++entity2)
		{
			comp::BoundingOrientedBox obb2;
			//comp::BoundingSphere sphere2;

			obb2 = pack.get<comp::BoundingOrientedBox>(*entity2);
			//sphere2 = viewSphere.get<comp::BoundingSphere>(*entity2);
			
			//Check collision with all colliders in the scene
			if (obb1.Intersects(obb2))
			{
				scene.publish<ESceneCollision>(*entity1, *entity2);
			}
					
			//else if(obb1.Intersects(sphere2))
			//	scene.publish<ESceneCollision>(*entity1, *entity2);
			//else if(sphere1.Intersects(obb2))
				//scene.publish<ESceneCollision>(*entity1, *entity2);
			//else if(sphere1.Intersects(sphere2))
				//scene.publish<ESceneCollision>(*entity1, *entity2);
		}
	}
}


