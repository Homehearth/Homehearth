#include "GameSystems.h"

#include "Tags.h"

using namespace network;
void GameSystems::UserInputSystem(Scene& scene, Client& client)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::Player>([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
		{
			velocity.vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
			velocity.vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
		});

	scene.ForEachComponent<comp::Attack>([&](comp::Attack attacker)
		{
			attacker.isAttacking = InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED);
			LOG_INFO("'Attack' input detected.");
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

void GameSystems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each Good Guy.
	scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<GOOD>>([&](Entity& player, comp::Attack& playerAttack, comp::Health& playerHealth, comp::Transform& playerTransform, comp::Tag<GOOD>&)
		{
			// For Each Bad Guy.
			scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<BAD>>([&](entt::entity enemy, comp::Attack& enemyAttack, comp::Health& enemyHealth, comp::Transform& enemyTransform, comp::Tag<BAD>&)
			{
				// Calculate the distance between player and enemy.
				const auto dist = (playerTransform.position - enemyTransform.position).Length();

				// Check if attack is in range.
				if(playerAttack.attackRange >= dist)
				{
					LOG_INFO("Player/Enemy is in attack range.");

					// Perform battle logic.
					if((enemyAttack.isAttacking & enemyHealth.isAlive) == TRUE)
					{
						playerHealth.currentHealth -= enemyAttack.attackDamage;
						LOG_INFO("Enemy attacked.");
					}

					if((playerAttack.isAttacking & playerHealth.isAlive) == TRUE)
					{
						enemyHealth.currentHealth -= playerAttack.attackDamage;
						LOG_INFO("Player attacked.");
					}
				}
			});
		});
}

//System to render collider mesh red if collider is colliding with another collider
void GameSystems::RenderIsCollidingSystem(Scene& scene)
{
	scene.ForEachComponent<comp::RenderableDebug>([&](entt::entity entity, comp::RenderableDebug& renderableDebug)
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
