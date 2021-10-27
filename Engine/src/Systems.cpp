#include "EnginePCH.h"
#include "Systems.h"

// TODO:
//#include "Tags.h"
const unsigned char GOOD = 4;
const unsigned char BAD = 8;

void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each Good Guy (Player).
	scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<GOOD>>([&](Entity& player, comp::Attack& playerAttack, comp::Health& playerHealth, comp::Transform& playerTransform, comp::Tag<GOOD>&)
		{
			if (playerHealth.isAlive)
			{
				// Decreases cooldown between attacks.
				if (playerAttack.cooldownTimer > 0.f)
					playerAttack.cooldownTimer -= 1.f * dt;

				// For Each Bad Guy (Enemy).
				scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<BAD>>([&](Entity& enemy, comp::Attack& enemyAttack, comp::Health& enemyHealth, comp::Transform& enemyTransform, comp::Tag<BAD>&)
					{
						if (enemyHealth.isAlive)
						{
							// Decreases cooldown between attacks
							if (enemyAttack.cooldownTimer > 0.f)
								enemyAttack.cooldownTimer -= 1.f * dt;

							//
							// PLAYER LOGIC
							//
							if (playerAttack.isAttacking)
							{
								if (!playerAttack.isRanged)
								{
									// Calculate the distance between player and enemy.
									const auto distToEnemy = (playerTransform.position - enemyTransform.position).Length();

									// Check if attack is in range.
									if (playerAttack.attackRange >= distToEnemy &&
										playerAttack.cooldownTimer <= 0.f)
									{
										Entity attackBox = scene.CreateEntity();
										attackBox.AddComponent<comp::BoundingOrientedBox>()->Center = playerTransform.position + ecs::GetForward(playerTransform);

										// Check if attack hit.
										CollisionSystem::Get().AddPair(attackBox, enemy);
										if (CollisionSystem::Get().IsColliding(attackBox, enemy))
										{
											// Perform battle logic.
											playerAttack.cooldownTimer = playerAttack.attackSpeed; //Set attacker on cooldown
											enemyHealth.currentHealth -= playerAttack.attackDamage; //Decrease health by the attackdamage
											LOG_INFO("Player %u: Attack landed.", player.GetComponent<comp::Network>()->id);
										}

										// Cleanup.
										CollisionSystem::Get().RemovePair(attackBox, enemy);
										attackBox.Destroy();
									}
								}
								else 
								{
									// Spawn projectile...
									Entity projectile = scene.CreateEntity();
									projectile.AddComponent<comp::Transform>()->position = playerTransform.position + ecs::GetForward(playerTransform);
									projectile.AddComponent<comp::BoundingOrientedBox>()->Center = playerTransform.position + ecs::GetForward(playerTransform);
									projectile.AddComponent<comp::Velocity>()->vel = ecs::GetForward(playerTransform) * playerAttack.attackSpeed;
									projectile.AddComponent<comp::Projectile>()->lifeTime = 10.f;
								}
								
								playerAttack.isAttacking = false;
							}


							//
							// ENEMY LOGIC
							//
							if (enemyAttack.isAttacking)
							{
								if (!enemyAttack.isRanged)
								{
									// Calculate the distance between enemy and player.
									const auto distToPlayer = (enemyTransform.position - playerTransform.position).Length();

									// Check if attack is in range.
									if (enemyAttack.attackRange >= distToPlayer &&
										enemyAttack.cooldownTimer <= 0.f)
									{
										Entity attackBox = scene.CreateEntity();
										attackBox.AddComponent<comp::BoundingOrientedBox>()->Center = enemyTransform.position + ecs::GetForward(enemyTransform);

										// Check if attack hit.
										CollisionSystem::Get().AddPair(attackBox, player);
										if (CollisionSystem::Get().IsColliding(attackBox, player))
										{
											// Perform battle logic.
											enemyAttack.cooldownTimer = enemyAttack.attackSpeed; //Set attacker on cooldown
											playerHealth.currentHealth -= enemyAttack.attackDamage; //Decrease health by the attackdamage
											LOG_INFO("Enemy %u: Attack landed.", enemy.GetComponent<comp::Network>()->id);
										}

										// Cleanup.
										CollisionSystem::Get().RemovePair(attackBox, player);
										attackBox.Destroy();
									}
								}
								else
								{
									// Spawn projectile...
								}

								enemyAttack.isAttacking = false;
							}
						}

					});
			}
		});
	

	//Health System
	scene.ForEachComponent<comp::Health>([&](Entity& Entity, comp::Health& Health)
		{
			//Check if something should be dead, and if so set isAlive to false
			if (Health.currentHealth <= 0)
			{
				LOG_INFO("Entity died");
				Health.isAlive = false;
				Entity.Destroy(); //Don't really know if this should be called here
			}
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
	scene.ForEachComponent<comp::Light>([&](Entity e, comp::Light light)
		{
			//If an Entity has both a Light and Transform component use Transform for position
			comp::Transform* t = e.GetComponent<comp::Transform>();
			if (t)
			{
				light.lightData.position = sm::Vector4(t->position.x, t->position.y, t->position.z, 1.f);
			}
			scene.GetLights()->EditLight(light.lightData, light.index);
		});

	
}
