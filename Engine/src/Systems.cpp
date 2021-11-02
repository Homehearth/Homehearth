#include "EnginePCH.h"
#include "Systems.h"


void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each entity that can attack.
	scene.ForEachComponent<comp::CombatStats, comp::Transform>([&](Entity entity, comp::CombatStats& stats, comp::Transform& transform)
		{
			// Decreases cooldown between attacks.
			if (stats.cooldownTimer > 0.f)
				stats.cooldownTimer -= dt;

			//
			// attack LOGIC
			//
			if (stats.isAttacking)
			{
				/*
				for (int i = 0; i < 100; i++)
				{
					Entity e = scene.CreateEntity();
					e.AddComponent<comp::Network>();
					e.AddComponent<comp::MeshName>("cube.obj");
					e.AddComponent<comp::Transform>()->position = transform.position + sm::Vector3(i) * 2;
				}
				*/
				//Creates an entity that's used to check collision if an attack lands.
				Entity attackCollider = scene.CreateEntity();
				attackCollider.AddComponent<comp::Transform>()->position = transform.position + stats.targetDir;
				attackCollider.AddComponent<comp::BoundingOrientedBox>()->Center = transform.position + stats.targetDir;
				comp::Attack* atk = attackCollider.AddComponent<comp::Attack>();
				atk->lifeTime = stats.attackLifeTime;
				atk->damage = stats.attackDamage;

				//If the attack is ranged add a velocity to the entity.
				if (stats.isRanged)
				{
					sm::Vector3 vel = stats.targetDir * 10.f; //CHANGE HERE WHEN FORWARD GETS FIXED!!!!!!
					attackCollider.AddComponent<comp::Velocity>()->vel = vel;
				}

				//DEBUG
				LOG_INFO("Attack Collider Created!");
				attackCollider.AddComponent<comp::Network>();
				//

				
				CollisionSystem::Get().AddOnCollision(attackCollider, [=](Entity other)
					{
						if (other == entity)
							return;
					
						
						comp::Health* otherHealth = other.GetComponent<comp::Health>();
						comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

						if (otherHealth)
						{
							otherHealth->currentHealth -= atk->damage;
							LOG_INFO("ATTACK COLLIDER HIT BAD GUY!");
							atk->lifeTime = 0.f;
						}
					});

				stats.cooldownTimer = stats.attackSpeed;								
				stats.isAttacking = false;
			}

		});
	

	//Health System
	scene.ForEachComponent<comp::Health>([&](Entity& entity, comp::Health& health)
		{
			//Check if something should be dead, and if so set isAlive to false
			if (health.currentHealth <= 0)
			{
				LOG_INFO("Entity died");
				health.isAlive = false;
				if (!entity.GetComponent<comp::Player>())
				{
					entity.Destroy();
				}

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
			transform.previousPosition = transform.position;
			transform.position += velocity.vel * dt;
		});
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	//BoundingOrientedBox
	scene.ForEachComponent<comp::Transform, comp::BoundingOrientedBox>([&, dt](comp::Transform& transform, comp::BoundingOrientedBox& obb)
		{
			obb.Center = transform.position;
			obb.Orientation = transform.rotation;
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
