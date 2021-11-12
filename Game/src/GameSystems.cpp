#include "GameSystems.h"
#include "EnginePCH.h"
#include "Healthbar.h"
#include "Game.h"

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
void GameSystems::CheckLOS(Game* game)
{
	// Shoot a ray from cameras position to players position
	Ray_t ray;
	ray.origin = game->GetCurrentScene()->GetCurrentCamera()->GetPosition();
	ray.dir = (game->m_players.at(game->m_localPID).GetComponent<comp::Transform>()->position - ray.origin);
	ray.dir.Normalize(ray.dir);

	for (int i = 0; i < game->m_LOSColliders.size(); i++)
	{
		if (ray.Intersects(game->m_LOSColliders[i].second))
		{
			for (int j = 0; j < game->m_models.at(game->m_LOSColliders[i].first).size(); j++)
			{
				game->m_models.at(game->m_LOSColliders[i].first)[j].GetComponent<comp::Renderable>()->isSolid = false;
			}
			break;
		}
	}
}