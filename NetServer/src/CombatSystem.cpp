#include "NetServerPCH.h"
#include "CombatSystem.h"

void CombatSystem::UpdateMelee(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::MeleeAttackAbility, comp::Transform>([&](Entity entity, comp::MeleeAttackAbility& stats, comp::Transform& transform)
		{
			sm::Vector3* updateTargetPoint = nullptr;

			UpdateTargetPoint(entity, updateTargetPoint);

		if (ecs::ReadyToUse(&stats, updateTargetPoint))
		{
			Entity attackEntity = CreateAttackEntity(entity, scene, &transform, &stats);
			AddCollisionMeleeBehavior(entity, attackEntity, scene);

			audio_t audio = {
				ESoundEvent::NONE,
				entity.GetComponent<comp::Transform>()->position,
				10.f,
				50.f,
				true,
				false,
				true,
				false,
			};

			if(entity.GetComponent<comp::Player>())
			{
				audio.type = ESoundEvent::Player_OnMeleeAttack;
			}
			else if(entity.GetComponent<comp::NPC>())
			{
				audio.type = ESoundEvent::Enemy_OnMeleeAttack;
			}

			scene.ForEachComponent<comp::Player>([&](Entity& playerEntity, comp::Player& player)
				{
					playerEntity.GetComponent<comp::AudioState>()->data.emplace(audio);
				});

		}
	});
}



void CombatSystem::UpdateRange(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::RangeAttackAbility, comp::Transform>([&](Entity entity, comp::RangeAttackAbility& stats, comp::Transform& transform)
		{
			sm::Vector3* updateTargetPoint = nullptr;

			UpdateTargetPoint(entity, updateTargetPoint);

		if (ecs::ReadyToUse(&stats, updateTargetPoint))
		{
			Entity attackEntity = CreateAttackEntity(entity, scene, &transform, &stats);
			AddCollisionRangeBehavior(entity, attackEntity, scene);

			audio_t audio = {
				ESoundEvent::NONE,
				entity.GetComponent<comp::Transform>()->position,
				10.f,
				100.f,
				true,
				false,
				true,
				false,
			};

			if (entity.GetComponent<comp::Player>())
			{
				audio.type = ESoundEvent::Player_OnRangeAttack;
				entity.GetComponent<comp::AudioState>()->data.emplace(audio);
			}
			else if (entity.GetComponent<comp::NPC>())
			{
				audio.type = ESoundEvent::Enemy_OnRangeAttack;
				entity.GetComponent<comp::AudioState>()->data.emplace(audio);
			}
		}
	});
}

void CombatSystem::UpdateTeleport(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::BlinkAbility, comp::Transform>([&](Entity entity, comp::BlinkAbility& teleportAbility, comp::Transform& transform)
		{
			PathFinderManager* pathFinderManager = Blackboard::Get().GetPathFindManager();
			sm::Vector3* targetPoint = nullptr;

			comp::Player* player = entity.GetComponent<comp::Player>();
			if (player)
			{
				targetPoint = &player->mousePoint; // only update targetPoint if this is a player
			}

			if (ecs::ReadyToUse(&teleportAbility, nullptr))
			{
				const float decreaseValue = 0.90f;
				if (targetPoint)
				{
					sm::Vector3 direction = *targetPoint - transform.position;
					direction.Normalize();
					direction *= teleportAbility.distance;

					bool hasSetTarget = false;
					while (!hasSetTarget && direction.Length() > pathFinderManager->GetNodeSize())
					{
						sm::Vector3 newPos = transform.position + direction;
						if (pathFinderManager->FindClosestNode(newPos)->reachable)
						{
							entity.GetComponent<comp::Transform>()->position = transform.position + direction;
							hasSetTarget = true;
							if (pathFinderManager->PlayerAStar(entity.GetComponent<comp::Transform>()->position))
							{
								player->reachable = true;
							}
						}
						else
						{
							//Lower length of vector by 10% every failed try
							direction *= decreaseValue;
						}
					}
				}
			}
		});
}

void CombatSystem::UpdateDash(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::DashAbility, comp::Transform>([&](Entity entity, comp::DashAbility& dashAbility, comp::Transform& transform)
		{
			PathFinderManager* pathFinderManager = Blackboard::Get().GetPathFindManager();
			sm::Vector3* targetPoint = nullptr;

			comp::Player* player = entity.GetComponent<comp::Player>();
			if (player)
			{
				targetPoint = &player->mousePoint; // only update targetPoint if this is a player
			}

			if (ecs::ReadyToUse(&dashAbility, nullptr))
			{
				//dashAbility.velocityBeforeDash = entity.GetComponent<comp::Velocity>()->vel;
				audio_t audio = {
					ESoundEvent::Player_OnCastDash,
					entity.GetComponent<comp::Transform>()->position,
					10.f,
					100.f,
					true,
					false,
					true,
					false,
				};

				entity.GetComponent<comp::AudioState>()->data.emplace(audio);
			}
			if (ecs::IsUsing(&dashAbility))
			{
				entity.GetComponent<comp::Velocity>()->vel = dashAbility.velocityBeforeDash * dashAbility.force;
			}
			else
			{
				dashAbility.velocityBeforeDash = entity.GetComponent<comp::Velocity>()->vel;
			}


		});
}


void CombatSystem::UpdateCombatSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	// For Each entity that can use melee attack.
	UpdateMelee(scene);

	//For each entity that can use range attack
	UpdateRange(scene);

	//For each entity that can use teleport
	UpdateTeleport(scene);

	//For each entity that can use Dash
	UpdateDash(scene);
}



void CombatSystem::UpdateTargetPoint(Entity entity, sm::Vector3* targetPoint)
{
	comp::Player* player = entity.GetComponent<comp::Player>();

	if (player)
	{
		targetPoint = &player->mousePoint; // only update targetPoint if this is a player
	}
}



Entity CombatSystem::CreateAttackEntity(Entity entity, HeadlessScene& scene, comp::Transform* transform, comp::MeleeAttackAbility* stats)
{
	//Creates an entity that's used to check collision if an attack lands.
	Entity attackEntity = scene.CreateEntity();

	comp::Transform* t = attackEntity.AddComponent<comp::Transform>();
	attackEntity.AddComponent<comp::Tag<TagType::DYNAMIC>>();
	attackEntity.AddComponent<comp::Tag<TagType::NO_RESPONSE>>();

	comp::SphereCollider* bos = attackEntity.AddComponent<comp::SphereCollider>();

	bos->Radius = stats->attackRange;
	float attackRangeMultiplier = 1.f;
	if (entity.GetComponent<comp::Player>())
	{
		attackRangeMultiplier = 1.3f;
	}

	sm::Vector3 targetDir = stats->targetPoint - transform->position;
	targetDir.Normalize();
	t->position = transform->position + targetDir * stats->attackRange * attackRangeMultiplier;
	t->position.y = bos->Radius;
	t->rotation = transform->rotation;

	comp::SelfDestruct* selfDestruct = attackEntity.AddComponent<comp::SelfDestruct>();
	selfDestruct->lifeTime = stats->lifetime;

	attackEntity.AddComponent<comp::Network>();

	return attackEntity;
}

Entity CombatSystem::CreateAttackEntity(Entity entity, HeadlessScene& scene, comp::Transform* transform,
	comp::RangeAttackAbility* stats)
{
	//Creates an entity that's used to check collision if an attack lands.
	Entity attackEntity = scene.CreateEntity();

	comp::Transform* t = attackEntity.AddComponent<comp::Transform>();
	attackEntity.AddComponent<comp::Tag<TagType::DYNAMIC>>();
	attackEntity.AddComponent<comp::Tag<TagType::NO_RESPONSE>>();

	comp::SphereCollider* bos = attackEntity.AddComponent<comp::SphereCollider>();

	bos->Radius = stats->projectileSize;


	sm::Vector3 targetDir = stats->targetPoint - transform->position;
	targetDir.Normalize();
	t->position = transform->position + targetDir * 0.5f + sm::Vector3(0, 3, 0);
	t->rotation = transform->rotation;

	bos->Center = t->position;

	comp::SelfDestruct* selfDestruct = attackEntity.AddComponent<comp::SelfDestruct>();
	selfDestruct->lifeTime = stats->lifetime;

	sm::Vector3 vel = targetDir * stats->projectileSpeed;
	attackEntity.AddComponent<comp::Velocity>()->vel = vel;
	attackEntity.AddComponent<comp::MeshName>()->name = NameType::MESH_SPHERE;

	attackEntity.AddComponent<comp::Network>();

	return attackEntity;
}



void CombatSystem::AddCollisionMeleeBehavior(Entity entity, Entity attackEntity, HeadlessScene& scene)
{
	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [entity, &scene](Entity thisEntity, Entity other)
		{
			// is caster already dead
			if (entity.IsNull())
			{
				thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
				return;
			}

			if (other == entity)
				return;

			tag_bits goodOrBad = TagType::GOOD | TagType::BAD;
			if ((entity.GetTags() & goodOrBad) ==
				(other.GetTags() & goodOrBad))
			{
				return; //these guys are on the same team
			}

			if ((entity.GetTags() & TagType::GOOD) && (other.GetTags() & TagType::DEFENCE)
				|| (entity.GetTags() & TagType::DEFENCE) && (other.GetTags() & TagType::GOOD))
			{
				return; //good vs defense are on the same team aswell
			}

			comp::Health* otherHealth = other.GetComponent<comp::Health>();
			comp::MeleeAttackAbility* attackAbility = entity.GetComponent<comp::MeleeAttackAbility>();

			audio_t audio = {
				ESoundEvent::NONE,
				entity.GetComponent<comp::Transform>()->position,
				1.f,
				100.f,
				false,
				false,
				false,
				false,
			};

			if (otherHealth && attackAbility)
			{
				otherHealth->currentHealth -= attackAbility->attackDamage;
				
				if (other.GetComponent<comp::Tag<TagType::DEFENCE>>())
				{
					//TODO: add building particles
				}
				else
				{
					// Blood particle
					if (other.GetComponent<comp::PARTICLEEMITTER>())
					{
						other.RemoveComponent<comp::PARTICLEEMITTER>();
					}
					other.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,6,0 }, 50, 5.f, PARTICLEMODE::BLOOD, 1.5f, 1.f, true);

					scene.publish<EComponentUpdated>(other, ecs::Component::PARTICLEMITTER);
				}
				
				// update Health on network
				scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);

				// Add some sound effects
				comp::AudioState* audioState = other.GetComponent<comp::AudioState>();
				if(other.GetComponent<comp::Player>())
				{
					audio.type = ESoundEvent::Player_OnDmgRecieved;
				}
				else if (other.GetComponent<comp::NPC>())
				{
					audio.type = ESoundEvent::Enemy_OnDmgRecieved;
				}
				else if (other.GetComponent<comp::Tag<STATIC>>() && entity.GetComponent<comp::Player>())
				{
					audio.type = ESoundEvent::Player_OnMeleeAttackHit;
				}

				if (audioState)
				{
					audioState->data.emplace(audio);
				}

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

					force.force = toOther;
					force.force *= attackAbility->attackDamage * 1.5f;

					force.isImpulse = true;
					force.drag = 0.0f;
					force.actingTime = 0.2f;

					p->forces.push_back(force);

					auto gravity = ecs::GetGravityForce();
					p->forces.push_back(gravity);
				}
			}
			return;
		});
}

void CombatSystem::AddCollisionRangeBehavior(Entity entity, Entity attackEntity, HeadlessScene& scene)
{
	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [entity, &scene](Entity thisEntity, Entity other)
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
			comp::RangeAttackAbility* attackAbility = entity.GetComponent<comp::RangeAttackAbility>();

			if (otherHealth && attackAbility)
			{
				otherHealth->currentHealth -= attackAbility->attackDamage;


				if (other.GetComponent<comp::Tag<TagType::DEFENCE>>())
				{
					//TODO: add building particles
				}
				else
				{
					// Blood particle
					if (other.GetComponent<comp::PARTICLEEMITTER>())
					{
						other.RemoveComponent<comp::PARTICLEEMITTER>();
					}
					other.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,6,0 }, 50, 5.f, PARTICLEMODE::BLOOD, 1.5f, 1.f, true);

					scene.publish<EComponentUpdated>(other, ecs::Component::PARTICLEMITTER);
				}


				// update Health on network
				scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);

				// Add some sound effects
				if (other.GetComponent<comp::Player>())
				{
					audio_t audio;
					audio.type = ESoundEvent::Player_OnDmgRecieved;
					audio.position = other.GetComponent<comp::Transform>()->position;
					audio.isUnique = false;
					audio.shouldBroadcast = true;
					audio.playLooped = false;
					audio.is3D = true;
					audio.volume = 10.f;
					other.GetComponent<comp::AudioState>()->data.emplace(audio);
				}

				thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;

				//Change animation when taken damage
				comp::AnimationState* anim = other.GetComponent<comp::AnimationState>();
				if (anim)
				{
					anim->toSend = EAnimationType::TAKE_DAMAGE;
				}


				comp::Velocity* attackVel = thisEntity.GetComponent<comp::Velocity>();
				if (attackVel)
				{
					comp::TemporaryPhysics* p = other.AddComponent<comp::TemporaryPhysics>();
					comp::TemporaryPhysics::Force force = {};
					force.force = attackVel->vel;
					force.actingTime = 0.7f;
					p->forces.push_back(force);
				}
				else
				{

					sm::Vector3 toOther = other.GetComponent<comp::Transform>()->position - entity.GetComponent<comp::Transform>()->position;
					toOther.Normalize();

					comp::TemporaryPhysics* p = other.AddComponent<comp::TemporaryPhysics>();
					comp::TemporaryPhysics::Force force = {};

					force.force = toOther + sm::Vector3(0, 1, 0);
					force.force *= attackAbility->attackDamage;

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
