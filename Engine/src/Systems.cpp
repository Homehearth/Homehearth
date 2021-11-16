#include "EnginePCH.h"
#include "Systems.h"
#include "Text.h"
#include "Components.h"
#include "Healthbar.h"

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






void Systems::UpdateAbilities(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();
	for (auto type : entt::resolve())
	{
		using namespace entt::literals;

		entt::id_type abilityType[] = { type.info().hash() };
		for (auto e : scene.GetRegistry()->runtime_view(std::cbegin(abilityType), std::cend(abilityType)))
		{
			Entity entity(*scene.GetRegistry(), e);
			auto instance = type.func("get"_hs).invoke({}, entity);
			
			comp::IAbility* ability = instance.try_cast<comp::IAbility>();
			if (!ability)
			{
				break; // if not of ability type move on to next type
			}

			// Decreases cooldown between attacks.
			if (ability->delayTimer > 0.f)
				ability->delayTimer -= dt;
					
			if (ability->useTimer > 0.f)
				ability->useTimer -= dt;

			if (ability->cooldownTimer > 0.f)
				ability->cooldownTimer -= dt;

			if (ability->cooldownTimer <= 0.f && ability->delayTimer <= 0.f && ability->useTimer <= 0.f)
				ability->isReady = true;

			comp::Velocity* vel = entity.GetComponent<comp::Velocity>();
			if (vel && ecs::IsUsing(ability))
			{
				vel->vel *= ability->movementSpeedAlt;
			}
		}
	}

}

void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();
	
	// For Each entity that can attack.
	scene.ForEachComponent<comp::AttackAbility, comp::Transform>([&](Entity entity, comp::AttackAbility& stats, comp::Transform& transform)
		{
			//
			// attack LOGIC
			//
			sm::Vector3* updateTargetPoint = nullptr;
			comp::Player* player = entity.GetComponent<comp::Player>();
			if (player)
			{
				updateTargetPoint = &player->mousePoint; // only update targetPoint if this is a player
			}
			if (ecs::ReadyToUse(&stats, updateTargetPoint))
			{
				//Creates an entity that's used to check collision if an attack lands.
				Entity attackCollider = scene.CreateEntity();
				
				comp::Transform* t = attackCollider.AddComponent<comp::Transform>();
				attackCollider.AddComponent<comp::Tag<TagType::DYNAMIC>>();

				comp::BoundingSphere* bos = attackCollider.AddComponent<comp::BoundingSphere>();

				bos->Radius = stats.attackRange;
				
				sm::Vector3 targetDir = stats.targetPoint - transform.position;
				targetDir.Normalize();
				t->position = transform.position + targetDir * stats.attackRange * 0.5f + sm::Vector3(0, 4, 0);
				t->rotation = transform.rotation;

				bos->Center = t->position;

				comp::SelfDestruct* selfDestruct = attackCollider.AddComponent<comp::SelfDestruct>();
				selfDestruct->lifeTime = stats.lifetime;

				//If the attack is ranged add a velocity to the entity.
				if (stats.isRanged)
				{
					sm::Vector3 vel = targetDir * stats.projectileSpeed;
					attackCollider.AddComponent<comp::Velocity>()->vel = vel;
					attackCollider.AddComponent<comp::MeshName>()->name = "Sphere.obj";
				}
				attackCollider.AddComponent<comp::Network>();

				
				CollisionSystem::Get().AddOnCollisionEnter(attackCollider, [entity, &scene](Entity thisEntity, Entity other)
					{
						// is caster already dead
						if (entity.IsNull())
						{
							thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
							return NO_RESPONSE;
						}

						if (other == entity)
							return NO_RESPONSE;

						tag_bits goodOrBad = TagType::GOOD | TagType::BAD;
						if ((entity.GetTags() & goodOrBad) ==
							(other.GetTags() & goodOrBad))
						{
							return NO_RESPONSE; //these guys are on the same team
						}

						comp::Health* otherHealth = other.GetComponent<comp::Health>();
						comp::AttackAbility* stats = entity.GetComponent<comp::AttackAbility>();

						if (otherHealth)
						{
							otherHealth->currentHealth -= stats->attackDamage;
							// update Health on network
							scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);

							thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;


							comp::Velocity* attackVel = thisEntity.GetComponent<comp::Velocity>();
							if (attackVel)
							{
								comp::TemporaryPhysics* p = other.AddComponent<comp::TemporaryPhysics>();
								comp::TemporaryPhysics::Force force = {};
								force.force = attackVel->vel;
								p->forces.push_back(force);
							}
							else
							{

								sm::Vector3 toOther = other.GetComponent<comp::Transform>()->position - entity.GetComponent<comp::Transform>()->position;
								toOther.Normalize();

								comp::TemporaryPhysics* p = other.AddComponent<comp::TemporaryPhysics>();
								comp::TemporaryPhysics::Force force = {};

								force.force = toOther + sm::Vector3(0, 1, 0);
								force.force *= stats->attackDamage;

								force.isImpulse = true;
								force.drag = 0.0f;
								force.actingTime = 0.7f;

								p->forces.push_back(force);

								auto gravity = ecs::GetGravityForce();
								p->forces.push_back(gravity);
							}
							
						}
						return NO_RESPONSE;
					});

			}

		});


}

void Systems::HealingSystem(HeadlessScene& scene, float dt)
{
	// HealAbility system
	scene.ForEachComponent<comp::HealAbility, comp::Player>([&](Entity entity, comp::HealAbility& ability, comp::Player& player)
		{
			if (ecs::ReadyToUse(&ability))
			{
				LOG_INFO("Used healing ability");
				Entity collider = scene.CreateEntity();
				comp::Transform* transform = collider.AddComponent<comp::Transform>();
				transform->position = entity.GetComponent<comp::Transform>()->position;
				transform->scale = sm::Vector3(2);
				
				comp::BoundingSphere* sphere = collider.AddComponent<comp::BoundingSphere>();
				sphere->Center = transform->position;
				sphere->Radius = 1.f;

				collider.AddComponent<comp::Tag<TagType::DYNAMIC>>();
				
				comp::Velocity* vel = collider.AddComponent<comp::Velocity>();
				vel->scaleVel = sm::Vector3(20);
				vel->applyToCollider = true;

				collider.AddComponent<comp::SelfDestruct>()->lifeTime = ability.lifetime;

				collider.AddComponent<comp::Network>();

				CollisionSystem::Get().AddOnCollisionEnter(collider, [&, entity](Entity thisEntity, Entity other)
					{
						if (entity.IsNull())
							return NO_RESPONSE;

						comp::Health* h = other.GetComponent<comp::Health>();
						if (h)
						{
							h->currentHealth += ability.healAmount;
							scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);
						}

						return NO_RESPONSE;
					});

			}
		});

}

void Systems::HealthSystem(HeadlessScene& scene, float dt, uint32_t& money_ref)
{
	//Entity destoys self if health <= 0
	scene.ForEachComponent<comp::Health>([&](Entity& entity, comp::Health& health)
		{
			//Check if something should be dead, and if so set isAlive to false
			if (health.currentHealth <= 0)
			{
				comp::Network* net = entity.GetComponent<comp::Network>();
				health.isAlive = false;
				// increase money
				if (entity.GetComponent<comp::NPC>())
				{
					money_ref += 2;
				}

				// if player
				comp::Player* p = entity.GetComponent<comp::Player>();
				if (p)
				{
					p->respawnTimer = 10.f;
					entity.RemoveComponent<comp::Tag<TagType::DYNAMIC>>();
				}
				else {
					entity.Destroy();
				}
				

			}
		});
}

void Systems::SelfDestructSystem(HeadlessScene& scene, float dt)
{
	//Entity destroys self after set time
	scene.ForEachComponent<comp::SelfDestruct>([&](Entity& ent, comp::SelfDestruct& s)
		{
			s.lifeTime -= dt;
			if (s.lifeTime <= 0)
			{
				ent.Destroy();
			}
		});
}

void Systems::MovementSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	//Transform

	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::TemporaryPhysics >([&](Entity e, comp::Transform& t, comp::Velocity& v, comp::TemporaryPhysics& p)
		{
			v.vel = v.oldVel; // ignore any changes made to velocity made this frame
			auto& it = p.forces.begin();
			while (it != p.forces.end())
			{
				comp::TemporaryPhysics::Force& f = *it;

				if (f.isImpulse)
				{
					if (f.wasApplied)
					{
						// Apply drag
						v.vel *= 1.0f - (dt * f.drag);
					}
					else
					{
						// Apply force once
						v.vel = f.force;
						f.wasApplied = true;
					}
				}
				else
				{
					// Apply constant force
					v.vel += f.force * dt * 2.f;
				}

				sm::Vector3 newPos = t.position + v.vel * dt;
				if (newPos.y < 0.0f)
				{
					v.vel.y = 0;
					f.force.y = 0;
				}

				if (f.force.Length() < 0.01f)
				{
					f.actingTime = 0.0f;
				}

				f.actingTime -= dt;
				if (f.actingTime <= 0.0f)
				{
					it = p.forces.erase(it);
					if (p.forces.size() == 0)
					{
						e.RemoveComponent<comp::TemporaryPhysics>();
						return;
					}
				}
				else
				{
					it++;
				}
			}
		});

	{
		PROFILE_SCOPE("Add Velocity to Transform");
		scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt]
		(Entity e, comp::Transform& transform, comp::Velocity& velocity)
			{

				if (velocity.vel.Length() > 0.01f)
				{
					e.UpdateNetwork();
				}

				transform.position += velocity.vel * dt;

				if (transform.position.y < 0.f)
				{
					transform.position.y = 0.f;
					velocity.vel.y = 0;
				}
				velocity.oldVel = velocity.vel; // updated old vel position

				
				if (velocity.scaleVel.Length() > 0.01f)
				{
					e.UpdateNetwork();
				}
				transform.scale += velocity.scaleVel * dt;
				velocity.oldScaleVel = velocity.scaleVel; // updated old vel scale

				if (velocity.applyToCollider)
				{					
					comp::BoundingOrientedBox* box = e.GetComponent<comp::BoundingOrientedBox>();
					if (box)
					{
						box->Extents = box->Extents + velocity.scaleVel * dt;
					}

					comp::BoundingSphere* bos = e.GetComponent<comp::BoundingSphere>();
					if (bos)
					{
						bos->Radius += velocity.scaleVel.x * dt;
					}

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
			/*obb.Orientation = transform.rotation;*/
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

void Systems::AISystem(HeadlessScene& scene, PathFinderManager* aiHandler)
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

			Node* closestNod = aiHandler->FindClosestNode(transformCurrentClosestPlayer->position);

			if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= npc.attackRange)
			{
				comp::AttackAbility* stats = entity.GetComponent<comp::AttackAbility>();

				stats->targetPoint = transformCurrentClosestPlayer->position;
				
				if (ecs::UseAbility(stats))
				{
					// Enemy Attacked
				};
			}


			switch (npc.state)
			{
			case comp::NPC::State::ASTAR:

				if(npc.path.size() > 0)
				{
					npc.currentNode = npc.path.back();
					if (velocityTowardsPlayer && npc.currentNode)
					{
						velocityTowardsPlayer->vel = npc.currentNode->position - transformNPC->position;
						velocityTowardsPlayer->vel.Normalize();
						velocityTowardsPlayer->vel *= npc.movementSpeed;
					}

					if(sm::Vector3::Distance(npc.currentNode->position, transformNPC->position) < 8.f)
					{
						npc.path.pop_back();
					}
					
				}
				else
				{
					npc.currentNode = aiHandler->FindClosestNode(transformNPC->position);
					aiHandler->AStarSearch(entity);
					velocityTowardsPlayer->vel = sm::Vector3(0.f,0.f,0.f);
				}


			}
		});
}
