#include "EnginePCH.h"
#include "Systems.h"


void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each Good Guy (Player).
	scene.ForEachComponent<comp::CombatStats, comp::Health, comp::Transform, comp::Player>([&](Entity& player, comp::CombatStats& playerStats, comp::Health& playerHealth, comp::Transform& playerTransform, comp::Player&)
		{
			if (playerHealth.isAlive)
			{
				// Decreases cooldown between attacks.
				if (playerStats.cooldownTimer > 0.f)
					playerStats.cooldownTimer -= 1.f * dt;

				// For Each Bad Guy (Enemy).
				scene.ForEachComponent<comp::CombatStats, comp::Health, comp::Transform, comp::Enemy>([&](Entity& enemy, comp::CombatStats& enemyStats, comp::Health& enemyHealth, comp::Transform& enemyTransform, comp::Enemy&)
					{
						if (enemyHealth.isAlive)
						{
							// Decreases cooldown between attacks
							if (enemyStats.cooldownTimer > 0.f)
								enemyStats.cooldownTimer -= 1.f * dt;

							//
							// PLAYER LOGIC
							//
							if (playerStats.isAttacking)
							{
								//Creates an entity that's used to check collision if an attack lands.
								Entity attackCollider = scene.CreateEntity();
								attackCollider.AddComponent<comp::Transform>()->position = playerTransform.position + ecs::GetForward(playerTransform) * -1;
								attackCollider.AddComponent<comp::BoundingOrientedBox>()->Center = playerTransform.position + ecs::GetForward(playerTransform) * -1;
								attackCollider.AddComponent<comp::Attack>()->lifeTime = playerStats.attackLifeTime;
								attackCollider.GetComponent<comp::Attack>()->damage = playerStats.attackDamage;

								LOG_INFO("Attack Collider Created!");

								//If the attack is ranged add a velocity to the entity.
								if (playerStats.isRanged)
								{
									sm::Vector3 vel = ecs::GetForward(playerTransform) * -10.f; //CHANGE HERE WHEN FORWARD GETS FIXED!!!!!!
									attackCollider.AddComponent<comp::Velocity>()->vel = vel;
								}

								CollisionSystem::Get().AddOnCollision(attackCollider, [&, attackCollider](Entity& enemyCol)
									{
										comp::Health* enemyHealth = enemyCol.GetComponent<comp::Health>();
										comp::Enemy* enemyTag = enemyCol.GetComponent<comp::Enemy>();
										comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

										if (enemyHealth && atk && enemyTag)
										{
											enemyHealth->currentHealth -= atk->damage;
											LOG_INFO("ATTACK COLLIDER HIT BAD GUY!");
											atk->lifeTime = 0.f;
										}
									});

								playerStats.cooldownTimer = playerStats.attackSpeed;								
								playerStats.isAttacking = false;
							}


							//
							// ENEMY LOGIC
							//
							if (enemyStats.isAttacking)
							{
								//Creates an entity that's used to check collision if an attack lands
								Entity attackCollider = scene.CreateEntity();
								attackCollider.AddComponent<comp::Transform>()->position = enemyTransform.position + ecs::GetForward(enemyTransform);
								attackCollider.AddComponent<comp::BoundingOrientedBox>()->Center = enemyTransform.position + ecs::GetForward(enemyTransform);
								attackCollider.AddComponent<comp::Attack>()->lifeTime = enemyStats.attackLifeTime;
								attackCollider.GetComponent<comp::Attack>()->damage = enemyStats.attackDamage;
								
								LOG_INFO("Attack Collider Created!");

								//If the attack is ranged add a velocity to the entity.
								if (enemyStats.isRanged)
								{
									sm::Vector3 vel = ecs::GetForward(enemyTransform) * -10.f; //CHANGE HERE WHEN FORWARD GETS FIXED!!!!!!
									attackCollider.AddComponent<comp::Velocity>()->vel = vel;
								}

								CollisionSystem::Get().AddOnCollision(attackCollider, [&, attackCollider](Entity& playerCol)
									{
										comp::Health* playerHealth = playerCol.GetComponent<comp::Health>();
										comp::Player* playerTag = playerCol.GetComponent<comp::Player>();
										comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

										if (playerHealth && atk && playerTag)
										{
											playerHealth->currentHealth -= atk->damage;
											LOG_INFO("ATTACK COLLIDER HIT GOOD GUY!");
											atk->lifeTime = 0.f;
										}
									});

								enemyStats.cooldownTimer = enemyStats.attackSpeed;
								enemyStats.isAttacking = false;
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
				Entity.Destroy();
			}
		});

	//Projectile Life System
	scene.ForEachComponent<comp::Attack>([&](Entity& ent, comp::Attack& Projectile) 
		{
			Projectile.lifeTime -= 1.f * dt;

			if (Projectile.lifeTime <= 0)
			{
				LOG_INFO("Attack Collider Destroyed");
				ent.Destroy();
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
void Systems::AISystem(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::NPC>([&](Entity entity, comp::NPC& npc)
	{
		comp::Transform* transform = entity.GetComponent<comp::Transform>();
		//transform->position += sm::Vector3(0.01f, 0.f, 1.f);
	});
}
