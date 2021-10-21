#include "EnginePCH.h"
#include "Systems.h"

// TODO:
//#include "Tags.h"
const unsigned char GOOD = 4;
const unsigned char BAD = 8;

void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each Good Guy.
	scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<GOOD>>([&](Entity& player, comp::Attack& playerAttack, comp::Health& playerHealth, comp::Transform& playerTransform, comp::Tag<GOOD>&)
		{
			// For Each Bad Guy.
			scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<BAD>>([&](Entity& enemy, comp::Attack& enemyAttack, comp::Health& enemyHealth, comp::Transform& enemyTransform, comp::Tag<BAD>&)
				{
					// Calculate the distance between player and enemy.
					const auto dist = (playerTransform.position - enemyTransform.position).Length();
					
					//Decreases cooldown between attacks
					if (playerAttack.cooldownTimer > 0.f)
						playerAttack.cooldownTimer -= 1.f * dt;
					if (enemyAttack.cooldownTimer > 0.f)
						enemyAttack.cooldownTimer -= 1.f * dt;

					// Check if attack is in range.
					if (playerAttack.attackRange >= dist && playerAttack.cooldownTimer < 0.f)
					{
						LOG_INFO("Player is in attack range.");

						// Perform battle logic.
						if ((playerAttack.isAttacking & playerHealth.isAlive) == TRUE)
						{
							playerAttack.cooldownTimer = playerAttack.attackSpeed;
							enemyHealth.currentHealth -= playerAttack.attackDamage;
							LOG_INFO("Player attacked.");
						}
					}

					// Check if attack is in range.
					if (enemyAttack.attackRange >= dist && enemyAttack.cooldownTimer < 0.f)
					{
						LOG_INFO("Enemy is in attack range.");

						// Perform battle logic.
						if ((enemyAttack.isAttacking & enemyHealth.isAlive) == TRUE)
						{
							enemyAttack.cooldownTimer = enemyAttack.attackSpeed;
							playerHealth.currentHealth -= enemyAttack.attackDamage;
							LOG_INFO("Enemy attacked.");
						}
					}
				});
		});
}

void Systems::MovementSystem(HeadlessScene& scene, float dt)
{
	//Transform
	scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt](comp::Transform& transform, comp::Velocity& velocity)
		{
			transform.position += velocity.vel * dt;
		});
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	//BoundingOrientedBox
	scene.ForEachComponent<comp::Transform, comp::BoundingOrientedBox>([&, dt](comp::Transform& transform, comp::BoundingOrientedBox& obb)
		{
			obb.Center = transform.position;
		});

	//BoundingSphere
	scene.ForEachComponent<comp::Transform, comp::BoundingSphere>([&, dt](comp::Transform& transform, comp::BoundingSphere& sphere)
		{
			sphere.Center = transform.position;
		});
}

void Systems::LightSystem(Scene& scene, float dt)
{
	//If you update the lightData update the info to the GPU
	scene.ForEachComponent<comp::Light>([&](comp::Light light)
		{
			scene.GetLights()->EditLight(light.lightData, light.index);
		});

	//If an Entity has both a Light and Transform component use Transform for position
	scene.ForEachComponent<comp::Transform, comp::Light>([&, dt](comp::Transform& transform, comp::Light& light)
		{
			light.lightData.position = sm::Vector4(transform.position.x, transform.position.y, transform.position.z, 1.f);
			scene.GetLights()->EditLight(light.lightData, light.index);
		});
	
}
