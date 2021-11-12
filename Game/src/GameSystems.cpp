#include "GameSystems.h"
#include "EnginePCH.h"
#include "Healthbar.h"

//System check if mouse ray intersects any of the box collider components in scene
void GameSystems::MRayIntersectBoxSystem(Scene& scene)
{
	scene.ForEachComponent<comp::BoundingOrientedBox, comp::Transform>([&](Entity entity, comp::BoundingOrientedBox& boxCollider, comp::Transform& transform)
		{
			//Collided with mouse TODO make it do someting?
			Ray_t ray = InputSystem::Get().GetMouseRay();

			if (ray.Intersects(boxCollider))
			{
				LOG_INFO("Mouseray HIT box detected!");
			}
		});
}

//System to render collider mesh red if collider is colliding with another collider
void GameSystems::RenderIsCollidingSystem(Scene& scene)
{
	scene.ForEachComponent<comp::RenderableDebug>([&](Entity entity, comp::RenderableDebug& renderableDebug)
		{
			//Collided with mouse TODO make it do someting?
			if (CollisionSystem::Get().GetCollisionCounts(entity) > 0)
			{
				renderableDebug.isColliding.hit = 1;
			}
			else
			{
				renderableDebug.isColliding.hit = 0;
			}
		});
}

// Set all the healthbars to players.
void GameSystems::UpdateHealthbar(Scene& scene)
{
	int i = 1;
	scene.ForEachComponent<comp::Health, comp::Player>([&](Entity e, comp::Health& health, const comp::Player& player) {
		// Safety check for now.
		if (i < 5)
		{
			rtd::Healthbar* healthbar = dynamic_cast<rtd::Healthbar*>(scene.GetCollection("player" + std::to_string(i) + "Info")->elements[0].get());
			if (healthbar)
			{
				healthbar->SetHealthVariable(e);
			}
		}
		i++;
		});
}

// Will check if a ray hits an object before it hits the player and if it does it will add the object to be rendered transparent
void GameSystems::CheckLOS(const sm::Vector3& camPos, const sm::Vector3& playerPos, const std::vector<dx::BoundingSphere>& mapColliders)
{
	// Shoot a ray from cameras position to players position
	Ray_t ray;
	ray.origin = camPos;
	ray.dir = (playerPos - ray.origin);
	ray.dir.Normalize(ray.dir);

	for (int i = 0; i < mapColliders.size(); i++)
	{
		if (ray.Intersects(mapColliders[i]))
		{
			//LOG_INFO("You are now behind an object and we cannot see you ROFL!");
			break;
		}
	}
}