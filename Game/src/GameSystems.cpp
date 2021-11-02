#include "GameSystems.h"
#include "EnginePCH.h"
#include "Healthbar.h"

using namespace network;


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

//System to render collider mesh red if collider is colliding with another collider
void GameSystems::RenderIsCollidingSystem(Scene& scene)
{
	scene.ForEachComponent<comp::RenderableDebug>([&](Entity entity, comp::RenderableDebug& renderableDebug)
		{
			//Collided with mouse TODO make it do someting?
			if(CollisionSystem::Get().getCollisionCounts(entity) > 0)
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
	scene.ForEachComponent<comp::Health, comp::Player>([&](Entity e,comp::Health& health, const comp::Player& player) {
		// Safety check for now.
		if (i < 5)
		{
			rtd::Healthbar* healthbar = dynamic_cast<rtd::Healthbar*>(scene.GetCollection("player" + std::to_string(i) + "Info")->elements[0].get());
			if (healthbar)
			{
				healthbar->SetHealthVariable(&health.currentHealth, health.maxHealth);
			}
		}
		});
}

