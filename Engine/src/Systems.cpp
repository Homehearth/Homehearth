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
	CombatSystem::UpdateCombatSystem(scene, dt);
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
				
				collider.AddComponent<comp::Tag<TagType::NO_RESPONSE>>();

				collider.AddComponent<comp::Network>();

				CollisionSystem::Get().AddOnCollisionEnter(collider, [&, entity](Entity thisEntity, Entity other)
					{
						if (entity.IsNull())
							return;

						comp::Health* h = other.GetComponent<comp::Health>();
						if (h)
						{
							h->currentHealth += ability.healAmount;
							scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);
						}

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
			else if (health.currentHealth > health.maxHealth)
			{
				health.currentHealth = health.maxHealth;
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