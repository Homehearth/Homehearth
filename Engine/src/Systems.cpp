#include "EnginePCH.h"
#include "Systems.h"

Entity FindClosestPlayer(HeadlessScene& scene, sm::Vector3 position, comp::NPC* npc)
{
	
	comp::Transform* transformCurrentClosest = nullptr;

	scene.ForEachComponent < comp::Player>([&](Entity& playerEntity, comp::Player& player)
	{
		if (!npc->currentClosest.IsNull())
		{
			transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
			comp::Transform* transformPlayer = playerEntity.GetComponent<comp::Transform>();
			if (sm::Vector3::Distance(transformPlayer->position, position) < sm::Vector3::Distance(transformCurrentClosest->position, position))
			{
				LOG_INFO("Switching player");
				npc->currentClosest = playerEntity;
				transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
			}
		}
		else
		{
			npc->currentClosest = playerEntity;
			transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
		}
	});

	return npc->currentClosest;
}





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
			if (stats.isAttacking && stats.cooldownTimer <= 0.f)
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
					attackCollider.AddComponent<comp::Network>();
				}

				
				CollisionSystem::Get().AddOnCollision(attackCollider, [=](Entity other)
					{
						if (other == entity)
							return;
					
						
						comp::Health* otherHealth = other.GetComponent<comp::Health>();
						comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

						if (otherHealth)
						{
							otherHealth->currentHealth -= atk->damage;

							atk->lifeTime = 0.f;
							comp::Velocity* attackVel = attackCollider.GetComponent<comp::Velocity>();
							if (attackVel)
							{
								other.AddComponent<comp::Force>()->force = attackVel->vel;
							}
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
		(Entity e, comp::Transform& transform, comp::Velocity& velocity)
			{
				transform.position += velocity.vel * dt;
				transform.position.y = 1.0f;
				if (velocity.vel.Length() > 0.01f)
				{
					e.UpdateNetwork();
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

void Systems::AISystem(HeadlessScene& scene, AIHandler* aiHandler)
{
	PROFILE_FUNCTION();

	scene.ForEachComponent<comp::NPC>([&](Entity entity, comp::NPC& npc)
	{
		comp::Transform* transformNPC = entity.GetComponent<comp::Transform>();
		Entity currentClosestPlayer;
		//npc.currentNode = FindClosestNode(scene, transformNPC->position);

		Entity closestPlayer = FindClosestPlayer(scene, transformNPC->position, &npc);
		comp::Velocity* velocityTowardsPlayer = entity.GetComponent<comp::Velocity>();
		comp::Transform* transformCurrentClosestPlayer = closestPlayer.GetComponent<comp::Transform>();
		if (npc.currentNode)
		{
			if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= 100.f && npc.state != comp::NPC::State::CHASE)
			{
				npc.state = comp::NPC::State::CHASE;
				LOG_INFO("Switching to CHASE State!");
			}
			else if ((sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) >= npc.attackRange + 100.f && npc.state != comp::NPC::State::ASTAR))
			{
				npc.state = comp::NPC::State::ASTAR;
				LOG_INFO("Switching to ASTAR State!");
			}

			else if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= npc.attackRange)
			{
				comp::CombatStats* stats = entity.GetComponent<comp::CombatStats>();

				stats->targetDir = transformCurrentClosestPlayer->position - transformNPC->position;
				stats->targetDir.Normalize();
				stats->isAttacking = true;
			}
		}
		else
			npc.state = comp::NPC::State::IDLE;

		npc.state = comp::NPC::State::CHASE;

		if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= npc.attackRange)
		{
			comp::CombatStats* stats = entity.GetComponent<comp::CombatStats>();

			stats->targetDir = transformCurrentClosestPlayer->position - transformNPC->position;
			stats->targetDir.Normalize();
			stats->targetDir *= 10.f;
			stats->isAttacking = true;
		}
		

		switch (npc.state)
		{
		case comp::NPC::State::CHASE:

			if (velocityTowardsPlayer)
			{
				velocityTowardsPlayer->vel = transformCurrentClosestPlayer->position - transformNPC->position;
				velocityTowardsPlayer->vel.Normalize();
				velocityTowardsPlayer->vel *= npc.movementSpeed;
			}
			break;
		case comp::NPC::State::ASTAR:
			if (aiHandler->ReachedNode(entity))
			{
				if (!npc.path.empty())
				{
					npc.currentNode = npc.path.at(0);
					npc.path.erase(npc.path.begin());
				}
				else
				{
					aiHandler->SetClosestNode(npc, transformNPC->position);
					aiHandler->AStarSearch(scene, entity);
				}
			}
			else
			{
				if (velocityTowardsPlayer && npc.currentNode)
				{
					velocityTowardsPlayer->vel = npc.currentNode->position - transformNPC->position;
					velocityTowardsPlayer->vel.Normalize();
					velocityTowardsPlayer->vel *= npc.movementSpeed;
				}				
			}
			break;
		case comp::NPC::State::IDLE:
			velocityTowardsPlayer->vel = { 0.f, 0.f, 0.f };
			break;
		}
	});
}
