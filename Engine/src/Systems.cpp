#include "EnginePCH.h"
#include "Text.h"
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
				transform->syncColliderScale = true;

				comp::SphereCollider* sphere = collider.AddComponent<comp::SphereCollider>();
				sphere->Center = transform->position;

				collider.AddComponent<comp::ParticleEmitter>(sm::Vector3{ 0,0,0 }, 200, 2.f, ParticleMode::MAGEHEAL, 30.f, 70.f, false);

				collider.AddComponent<comp::Tag<TagType::DYNAMIC>>();

				audio_t audio = {
					ESoundEvent::Player_OnHealing,
					entity.GetComponent<comp::Transform>()->position,
					1.0f,
					100.f,
					true,
					false,
					true,
					false,
				};

				// Send audio to healer.
				entity.GetComponent<comp::AudioState>()->data.emplace(audio);

				comp::BezierAnimation* a = collider.AddComponent<comp::BezierAnimation>();
				a->speed = 0.5f;
				a->scalePoints.push_back(transform->scale);
				a->scalePoints.push_back(transform->scale + sm::Vector3(ability.range));

				collider.AddComponent<comp::SelfDestruct>()->lifeTime = ability.lifetime;

				collider.AddComponent<comp::Tag<TagType::NO_RESPONSE>>();

				collider.AddComponent<comp::Network>();

				CollisionSystem::Get().AddOnCollisionEnter(collider, [&, entity](Entity thisEntity, Entity other)
					{
						if (entity.IsNull())
							return;

						comp::Player* p = other.GetComponent<comp::Player>();
						if (p)
						{
							scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);
							// Send audio to everyone effected by heal.
							other.GetComponent<comp::AudioState>()->data.emplace(audio);
							comp::Health* h = other.GetComponent<comp::Health>();

							if (h)
							{
								h->currentHealth += ability.healAmount;
								scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);
							}
						}
					});
			}
		});

}

void Systems::HeroLeapSystem(HeadlessScene& scene, float dt)
{
	scene.ForEachComponent<comp::Transform, comp::HeroLeapAbility>([&](Entity e, comp::Transform& t, comp::HeroLeapAbility& ability)
		{
			sm::Vector3* point = nullptr;
			comp::Player* player = e.GetComponent<comp::Player>();
			if (player)
			{
				point = &player->mousePoint;
			}

			if (ecs::ReadyToUse(&ability, point))
			{
				comp::BezierAnimation* a = e.AddComponent<comp::BezierAnimation>();
				a->translationPoints.push_back(t.position);
				sm::Vector3 toTarget = ability.targetPoint - t.position;
				float distance = toTarget.Length();
				toTarget.Normalize();
				distance = min(distance, ability.maxRange);
				toTarget *= distance;

				a->translationPoints.push_back(t.position);
				a->translationPoints.push_back(t.position + sm::Vector3(0, 40, 0));
				a->translationPoints.push_back(t.position + toTarget);
				a->speed = ability.useTime;

				a->onFinish = [&]()
				{
					Entity collider = scene.CreateEntity();
					collider.AddComponent<comp::Transform>()->position = t.position;
					comp::SphereCollider* sphere = collider.AddComponent<comp::SphereCollider>();
					sphere->Center = t.position;
					sphere->Radius = ability.damageRadius;

					collider.AddComponent<comp::SelfDestruct>()->lifeTime = ability.lifetime;
					collider.AddComponent<comp::Network>();
					collider.AddComponent<comp::Tag<TagType::DYNAMIC>>();
					collider.AddComponent<comp::Tag<TagType::NO_RESPONSE>>();

					CollisionSystem::Get().AddOnCollisionEnter(collider, [=, &scene](Entity thisEntity, Entity other)
						{
							// is caster already dead
							if (e.IsNull())
							{
								thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
								return;
							}

							if (other == e)
								return;

							tag_bits goodOrBad = TagType::GOOD | TagType::BAD;
							if ((e.GetTags() & goodOrBad) ==
								(other.GetTags() & goodOrBad))
							{
								return; //these guys are on the same team
							}

							comp::Health* otherHealth = other.GetComponent<comp::Health>();
							comp::HeroLeapAbility* ability = e.GetComponent<comp::HeroLeapAbility>();

							if (otherHealth)
							{
								otherHealth->currentHealth -= ability->damage;
								// update Health on network
								scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);

								sm::Vector3 toOther = other.GetComponent<comp::Transform>()->position - thisEntity.GetComponent<comp::Transform>()->position;
								toOther.Normalize();

								comp::TemporaryPhysics* p = other.AddComponent<comp::TemporaryPhysics>();
								comp::TemporaryPhysics::Force force = {};

								force.force = toOther + sm::Vector3(0, 5, 0);
								force.force *= ability->damage;

								force.isImpulse = true;
								force.drag = 0.0f;
								force.actingTime = 0.7f;

								p->forces.push_back(force);

								auto gravity = ecs::GetGravityForce();
								p->forces.push_back(gravity);
							}
						});
				};
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
				if(s.onDestruct)
					s.onDestruct();

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
			//Don't update villager that is in hiding
			comp::Villager* villager = e.GetComponent<comp::Villager>();
			if (villager != nullptr && villager->isHiding)
			{
				return;
			}

			v.vel = v.oldVel; // ignore any changes made to velocity made this frame
			auto it = p.forces.begin();
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
				if (newPos.y < 0.75f)
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

				//Don't update villager that is in hiding
				comp::Villager* villager = e.GetComponent<comp::Villager>();
				if (villager != nullptr && villager->isHiding)
				{
					return;
				}

				if (velocity.vel.Length() > 0.01f)
				{
					e.UpdateNetwork();
				}

				transform.position += velocity.vel * dt;

				if (transform.position.y < 0.75f)
				{
					transform.position.y = 0.75f;
					velocity.vel.y = 0;
				}
				velocity.oldVel = velocity.vel; // updated old vel position
			});
	}
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	//BoundingOrientedBox
	/*scene.ForEachComponent<comp::Transform, comp::OrientedBoxCollider>([&, dt]
	(Entity entity, comp::Transform& transform, comp::OrientedBoxCollider& obb)
		{
			//If its not a house update obb!
			if (!entity.GetComponent<comp::House>())
			{


				obb.Center = transform.position;
				if (transform.syncColliderScale)
					obb.Extents = transform.scale;
			}
		});*/

	//BoundingSphere
	scene.ForEachComponent<comp::Transform, comp::SphereCollider>([&, dt]
	(comp::Transform& transform, comp::SphereCollider& sphere)
		{
			sphere.Center = transform.position;
			if (transform.syncColliderScale)
				sphere.Radius = transform.scale.x;

		});
}

void Systems::LightSystem(Scene& scene, float dt)
{
	//If you update the lightData update the info to the GPU
	scene.ForEachComponent<comp::Light>([&](Entity e, comp::Light& light)
		{
			//If an Entity has both a Light and Transform component use Transform for position
			comp::Transform* t = e.GetComponent<comp::Transform>();
			if (t)
			{
				light.lightData.position = sm::Vector4(t->position.x, t->position.y, t->position.z, 1.f);
			}

			if (light.lightData.type == TypeLight::POINT && light.lightData.enabled)
			{
				if (light.enabledTimer > 0.f)
				{
					light.enabledTimer -= dt;
					light.lightData.intensity = util::Lerp(light.lightData.intensity, 0.3f, dt);
				}
				else
				{
					if (light.flickerTimer >= light.maxFlickerTime)
						light.increase = false;
					else if (light.flickerTimer <= 0.f)
					{
						light.increase = true;
						light.maxFlickerTime = (float)(rand() % 10 + 1) / 10.f;
					}

					if (light.increase)
						light.flickerTimer += dt * (rand() % 2 + 1);
					else
						light.flickerTimer -= dt * (rand() % 2 + 1);

					light.lightData.intensity = util::Lerp(0.5f, 0.7f, light.flickerTimer);
				}

			}
			else if (light.lightData.type == TypeLight::POINT && !light.lightData.enabled)
				light.enabledTimer = 1.f;

			scene.GetLights()->EditLight(light.lightData, light.index);
		});
}

void Systems::TransformAnimationSystem(HeadlessScene& scene, float dt)
{
	scene.ForEachComponent<comp::Transform, comp::BezierAnimation>([&](Entity e, comp::Transform& t, comp::BezierAnimation& a)
		{
			a.time += dt * (1.f / a.speed);

			if (a.translationPoints.size() > 0)
				t.position = util::BezierCurve(a.translationPoints, a.time);

			if (a.scalePoints.size() > 0)
				t.scale = util::BezierCurve(a.scalePoints, a.time);

			if (a.rotationPoints.size() > 0)
				t.rotation = util::BezierCurve(a.rotationPoints, a.time);

			e.UpdateNetwork();

			if (a.time > 1.0f)
			{
				if (a.onFinish)
					a.onFinish();

				a.time = 0.0f;
				if (!a.loop)
				{
					e.RemoveComponent<comp::BezierAnimation>();
				}
			}
		});
}

void Systems::UpdateDynamicQT(HeadlessScene& scene, QuadTree* qtDynamic)
{
	scene.ForEachComponent<comp::SphereCollider>([&](Entity& e, comp::SphereCollider& bs)
		{
			qtDynamic->Insert(e);
		});
}

void Systems::CheckCollisions(HeadlessScene& scene, float dt)
{
	scene.ForEachComponent<comp::SphereCollider>([&](Entity& e1, comp::SphereCollider& s)
		{
			for (auto e2 : s.list)
			{
				if (e1 != e2)
				{
					CollisionInfo_t collisionInfo = CollisionSystem::Get().Intersection(e1, e2);

					if (!e2.GetComponent<comp::Tag<TagType::STATIC>>())
					{
						comp::SphereCollider* s2 = e2.GetComponent<comp::SphereCollider>();
						if (s2)
						{
							s2->list.erase(e1);
						}
					}

					if (collisionInfo.hasCollided)
					{
						if (CollisionSystem::Get().AddPair(e1, e2))
						{
							CollisionSystem::Get().OnCollisionEnter(e1, e2);
						}

						CollisionSystem::Get().OnCollision(e1, e2);

						if (!e1.HasComponent<comp::Tag<TagType::NO_RESPONSE>>() && !e2.HasComponent<comp::Tag<TagType::NO_RESPONSE>>())
						{
							CollisionSystem::Get().CollisionResponse(collisionInfo, e1, e2);
						}
					}
					else
					{
						if (CollisionSystem::Get().RemovePair(e1, e2))
						{
							CollisionSystem::Get().OnCollisionExit(e1, e2);
						}
					}
				}
			}
		});
}

void Systems::FetchCollidingList(HeadlessScene& scene, QuadTree* qt, QuadTree* qtDynamic)
{
	scene.ForEachComponent<comp::SphereCollider>([&](Entity& e, comp::SphereCollider& s)
		{
			qt->Query(s.list, s);
			qtDynamic->Query(s.list, s);
		});
}

void Systems::ClearCollidingList(HeadlessScene& scene, QuadTree* qtDynamic)
{
	scene.ForEachComponent<comp::SphereCollider>([&](Entity& e, comp::SphereCollider& s)
		{
			s.list.clear();
		});

	qtDynamic->Clear();
}