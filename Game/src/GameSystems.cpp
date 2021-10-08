#include "GameSystems.h"

using namespace network;
void GameSystems::UserInputSystem(Scene& scene, Client& client)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::Player>([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
		{
			velocity.vel.z = -InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
			velocity.vel.x = -InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
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

void GameSystems::CollisionUpdateSystem(Scene& scene)
{
	//OBB collision on OBB & Sphere
	scene.ForEachComponent<comp::BoundingOrientedBox>([&](Entity entity, comp::BoundingOrientedBox& boundingBox)
		{
			scene.ForEachComponent<comp::BoundingOrientedBox>([&](Entity entity2, comp::BoundingOrientedBox& boundingBox2)
				{
					if (entity != entity2 && boundingBox.Intersects(boundingBox2))
					{
						scene.publish<ESceneCollision>(entity, entity2);
					}
				});

			scene.ForEachComponent<comp::BoundingSphere>([&](Entity entity2, comp::BoundingSphere& boundingsphere)
				{
					if (entity != entity2 && boundingBox.Intersects(boundingsphere))
					{
						scene.publish<ESceneCollision>(entity, entity2);
					}
				});
		});
	
	//Sphere collision on OBB & Sphere
	//TODO 
}


