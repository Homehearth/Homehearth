#include "EnginePCH.h"
#include "Systems.h"


void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();
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

				//Creates an entity that's used to check collision if an attack lands.
				Entity attackCollider = scene.CreateEntity();
				attackCollider.AddComponent<comp::Transform>()->position = transform.position + stats.targetDir;
				//attackCollider.AddComponent<comp::Tag<TagType::DYNAMIC>>();
				attackCollider.AddComponent<comp::BoundingOrientedBox>()->Center = transform.position + stats.targetDir;
				comp::Attack* atk = attackCollider.AddComponent<comp::Attack>();
				atk->lifeTime = stats.attackLifeTime;
				atk->damage = stats.attackDamage;

				//If the attack is ranged add a velocity to the entity.
				if (stats.isRanged)
				{
					sm::Vector3 vel = stats.targetDir * stats.projectileSpeed;
					attackCollider.AddComponent<comp::Velocity>()->vel = vel;

					attackCollider.AddComponent<comp::MeshName>()->name = "Sphere.obj";
				}
#ifdef _DEBUG
				LOG_INFO("Attack Collider Created!");
#endif
				
				attackCollider.AddComponent<comp::Network>();
				//DEBUG


				
				CollisionSystem::Get().AddOnCollision(attackCollider, [=](Entity other)
					{
						if (other == entity)
							return;
					
						
						comp::Health* otherHealth = other.GetComponent<comp::Health>();
						comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

						if (otherHealth)
						{
							otherHealth->currentHealth -= atk->damage;
#ifdef _DEBUG
							LOG_INFO("ATTACK COLLIDER HIT BAD GUY!");
#endif
							atk->lifeTime = 0.f;
							comp::Velocity* attackVel = attackCollider.GetComponent<comp::Velocity>();
							
							other.AddComponent<comp::Force>()->force = attackVel->vel;

							other.UpdateNetwork();
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
				comp::Network* net = entity.GetComponent<comp::Network>();
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
#ifdef _DEBUG
				LOG_INFO("Attack Collider Destroyed");
#endif
				ent.Destroy();
			}
		});
}

void Systems::MovementSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	//Transform

	scene.ForEachComponent<comp::Velocity, comp::Force>([&](Entity e, comp::Velocity& v, comp::Force& f)
		{
			if (f.wasApplied)
			{
				v.vel *= 1.0f - (dt * 10.f);
				f.actingTime -= dt;
				if (f.actingTime <= 0.0f)
				{
					e.RemoveComponent<comp::Force>();
				}
			}
			else
			{
				v.vel = f.force;
				f.wasApplied = true;
			}
		});

	{
		PROFILE_SCOPE("Add Velocity to Transform");
		scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt]
		(comp::Transform& transform, comp::Velocity& velocity)
			{
				transform.position += velocity.vel * dt;
				transform.position.y = 1.0f;
			
			});
	}
	{
		PROFILE_SCOPE("Update Transforms");
		scene.ForEachComponent<comp::Transform, comp::Network>([](Entity e, comp::Transform& t, comp::Network&) 
			{
				if (t.previousPosition != t.position)
				{
					e.UpdateNetwork();
					t.previousPosition = t.position;
				}
			});
	}
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	//BoundingOrientedBox
	scene.ForEachComponent<comp::Transform, comp::BoundingOrientedBox>([&, dt]
	(comp::Transform& transform, comp::BoundingOrientedBox& obb)
		{
			obb.Center = transform.position;
			obb.Orientation = transform.rotation;
		});

	//BoundingSphere
	scene.ForEachComponent<comp::Transform, comp::BoundingSphere>([&, dt]
	(comp::Transform& transform, comp::BoundingSphere& sphere)
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
