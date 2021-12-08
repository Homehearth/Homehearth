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
				

				//AddCollisionMeleeBehavior(entity, attackEntity, scene);

				audio_t audio = {
					ESoundEvent::NONE,
					entity.GetComponent<comp::Transform>()->position,
					1.f,
					50.f,
					true,
					false,
					true,
					false,
				};

				if (entity.GetComponent<comp::Player>())
				{
					audio.type = ESoundEvent::Player_OnMeleeAttack;
				}
				else if (entity.GetComponent<comp::NPC>())
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
				//AddCollisionRangeBehavior(entity, attackEntity, scene);

				audio_t audio = {
					ESoundEvent::NONE,
					entity.GetComponent<comp::Transform>()->position,
					1.f,
					100.f,
					true,
					false,
					true,
					false,
				};

				if (entity.GetComponent<comp::Player>())
				{
					audio.type = ESoundEvent::Player_OnRangeAttack;
				}
				else if (entity.GetComponent<comp::NPC>())
				{
					audio.type = ESoundEvent::Enemy_OnRangeAttack;
				}

				scene.ForEachComponent<comp::Player>([&](Entity& playerEntity, comp::Player& player)
					{
						playerEntity.GetComponent<comp::AudioState>()->data.emplace(audio);
					});
			}
		});
}

void CombatSystem::UpdateTeleport(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::BlinkAbility, comp::Transform>([&](Entity entity, comp::BlinkAbility& teleportAbility, comp::Transform& transform)
		{
			PathFinderManager* pathFinderManager = Blackboard::Get().GetPathFindManager();

			comp::Player* p = entity.GetComponent<comp::Player>();

			if (ecs::ReadyToUse(&teleportAbility, nullptr))
			{
				const float decreaseValue = 0.90f;
				if (p)
				{
					sm::Vector3 dir = ecs::GetForward(transform) * teleportAbility.distance;

					bool hasSetTarget = false;
					while (!hasSetTarget && dir.Length() > pathFinderManager->GetNodeSize())
					{
						sm::Vector3 newPos = transform.position + dir;
						if (pathFinderManager->FindClosestNode(newPos)->reachable)
						{
							entity.GetComponent<comp::Transform>()->position = transform.position + dir;
							hasSetTarget = true;
							entity.UpdateNetwork();
							if (pathFinderManager->PlayerAStar(entity.GetComponent<comp::Transform>()->position))
							{
								p->reachable = true;
							}

							audio_t audio = {
								ESoundEvent::Player_OnCastBlink,
								entity.GetComponent<comp::Transform>()->position,
								1.0f,
								250.f,
								true,
								false,
								true,
								false,
							};
							entity.GetComponent<comp::AudioState>()->data.emplace(audio);

						}
						else
						{
							//Lower length of vector by 10% every failed try
							dir *= decreaseValue;
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
			// Has to run to reset the ability

			if (ecs::ReadyToUse(&dashAbility, nullptr))
			{
				audio_t audio = {
					ESoundEvent::Player_OnCastDash,
					entity.GetComponent<comp::Transform>()->position,
					1.f,
					100.f,
					true,
					false,
					true,
					false,
				};

				entity.GetComponent<comp::AudioState>()->data.emplace(audio);
				comp::Player* p = entity.GetComponent<comp::Player>();
				if (p)
				{
					sm::Vector3 dir = ecs::GetForward(transform);

					dashAbility.velocityBeforeDash = dir * p->runSpeed;
				}
			}

			if (ecs::IsUsing(&dashAbility))
			{
				entity.GetComponent<comp::Velocity>()->vel = dashAbility.velocityBeforeDash * dashAbility.force;

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

	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [=, &scene](Entity thisEntity, Entity other)
		{
			comp::MeleeAttackAbility* ability = nullptr;
			if (!entity.IsNull())
				ability = entity.GetComponent<comp::MeleeAttackAbility>();
			if(ability)
				DoDamage(scene, entity, thisEntity, other, ability->attackDamage, 40.0f, AttackType::MELEE);

		});

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
	t->position = transform->position + targetDir * 10.f + sm::Vector3(0, 5, 0);
	t->rotation = transform->rotation;

	bos->Center = t->position;
	
	comp::SelfDestruct* selfDestruct = attackEntity.AddComponent<comp::SelfDestruct>();
	selfDestruct->lifeTime = 1000.f;
	selfDestruct->onDestruct = [&, entity, attackEntity]()
	{
		if (entity.IsNull())
			return;
		comp::AudioState* audioState = entity.GetComponent<comp::AudioState>();
		if (audioState)
		{
			audio_t audio = {};
			audio.position = attackEntity.GetComponent<comp::Transform>()->position;
			audio.is3D = true;
			audio.isUnique = false;
			audio.minDistance = 100.0f;
			audio.playLooped = false;
			audio.shouldBroadcast = true;
			audio.volume = 1.0f;
			audio.type = ESoundEvent::Player_OnRangeAttackHit;

			audioState->data.emplace(audio);
		}
		comp::RangeAttackAbility* ability = entity.GetComponent<comp::RangeAttackAbility>();
		Entity explosion = CreateAreaAttackCollider(scene, attackEntity.GetComponent<comp::Transform>()->position, ability->attackRange);

		explosion.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,0,0 }, 200, 6.f, PARTICLEMODE::EXPLOSION, 2.0f, 40.f, false);


		CollisionSystem::Get().AddOnCollisionEnter(explosion, [=, &scene](Entity expl, Entity other)
			{
				comp::MeleeAttackAbility* ability = nullptr;
				if (!entity.IsNull())
					ability = entity.GetComponent<comp::MeleeAttackAbility>();
				if (ability)
					DoDamage(scene, entity, expl, other, ability->attackDamage, 40.0f, AttackType::RANGE);
			});
	};
	
	attackEntity.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,0,0 }, 200, 1.f, PARTICLEMODE::MAGERANGE, 1.7f, 1.f, false);

	attackEntity.AddComponent<comp::Network>();

	comp::BezierAnimation* a = attackEntity.AddComponent<comp::BezierAnimation>();
	sm::Vector3 toTarget = stats->targetPoint - t->position;
	toTarget.y = 0.0f;
	float len = toTarget.Length();
	toTarget /= len;
	toTarget *= (len * 0.5f);
	toTarget.y += len * 0.6f;
	sm::Vector3 center = t->position + toTarget;

	a->translationPoints.push_back(t->position);
	a->translationPoints.push_back(center);
	a->translationPoints.push_back(stats->targetPoint);
	a->speed = 2 * toTarget.y / stats->projectileSpeed;

	a->onFinish = [&, attackEntity, entity]() mutable
	{
		attackEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.0f;	
	};

	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [=, &scene](Entity thisEntity, Entity)
		{
			thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.0f;
		});


	return attackEntity;
}



void CombatSystem::AddCollisionMeleeBehavior(Entity entity, Entity attackEntity, HeadlessScene& scene)
{
	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [attackEntity, entity, &scene](Entity thisEntity, Entity other)
		{
			// is caster already dead
			if (entity.IsNull())
			{
				thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
				return;
			}

			if (other == entity)
				return;

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

			// Map bounds
			if (other.GetTags() & STATIC && !other.GetComponent<comp::House>())
			{
				return;
			}

			tag_bits goodOrBad = TagType::GOOD | TagType::BAD;

			bool sameTeam = false;
			
			if ((entity.GetTags() & GOOD) == (other.GetTags() & GOOD))
			{
				sameTeam = true;
			}
			// Add some sound effects
			comp::AudioState* audioState = entity.GetComponent<comp::AudioState>();
			if (other.GetComponent<comp::Player>())
			{
				audio.type = ESoundEvent::Player_OnDmgRecieved;
			}
			else if (other.GetComponent<comp::NPC>())
			{
				audio.type = ESoundEvent::Enemy_OnDmgRecieved;
			}
			else if (!sameTeam && (attackEntity.GetTags() & NO_RESPONSE) != (other.GetTags() & NO_RESPONSE) || other.GetComponent<comp::House>())
			{
				audio.type = ESoundEvent::Player_OnMeleeAttackHit;
				audio.shouldBroadcast = true;
				audio.is3D = true;
			}

			if (audioState)
			{
				audioState->data.emplace(audio);
			}

			if (sameTeam)
			{
				return; //these guys are on the same team
			}

			if ((entity.GetTags() & GOOD) && (other.GetTags() & DEFENCE))
			{
				return; //good vs defense are on the same team aswell
			}

			if (attackAbility)
			{
				comp::Health* otherHealth = other.GetComponent<comp::Health>();
				if (otherHealth && (entity.GetTags() & goodOrBad) != (other.GetTags() & goodOrBad))
				{
					otherHealth->currentHealth -= attackAbility->attackDamage;
					// update Health on network
					scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);

					// Blood particle
					if (other.GetComponent<comp::PARTICLEEMITTER>())
					{
						other.RemoveComponent<comp::PARTICLEEMITTER>();
					}
					other.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,6,0 }, 50, 5.f, PARTICLEMODE::BLOOD, 1.5f, 1.f, true);

					scene.publish<EComponentUpdated>(other, ecs::Component::PARTICLEMITTER);
				}

				//Smoke particles
				if (other.GetComponent<comp::Tag<TagType::DEFENCE>>())
				{
					if (other.GetComponent<comp::PARTICLEEMITTER>())
					{
						other.RemoveComponent<comp::PARTICLEEMITTER>();
					}
					other.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,10,0 }, 50, 10.f, PARTICLEMODE::SMOKEAREA, 3.5f, 1.f, true);

					scene.publish<EComponentUpdated>(other, ecs::Component::PARTICLEMITTER);
				}

				thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;

				comp::Velocity* attackVel = thisEntity.GetComponent<comp::Velocity>();
				if (attackVel && otherHealth)
				{
					comp::TemporaryPhysics* p = other.AddComponent<comp::TemporaryPhysics>();
					comp::TemporaryPhysics::Force force = {};
					force.force = attackVel->vel;
					p->forces.push_back(force);
				}
				else if (otherHealth)
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
	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [attackEntity, entity, &scene](Entity thisEntity, Entity other)
		{
			// is caster already dead
			if (entity.IsNull())
			{
				//thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
				return;
			}

			if (other == entity)
				return;

			// Add some sound effects
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
			tag_bits goodOrBad = GOOD | BAD;

			bool sameTeam = false;

			if ((entity.GetTags() & goodOrBad) == (other.GetTags() & goodOrBad))
			{
				sameTeam = true;
			}

			comp::AudioState* audioState = entity.GetComponent<comp::AudioState>();
			if (other.GetComponent<comp::Player>())
			{
				audio.type = ESoundEvent::Player_OnDmgRecieved;
			}
			else if((!other.GetComponent<comp::Tag<STATIC>>() && (entity.GetTags() & goodOrBad) != (other.GetTags() & goodOrBad) && (attackEntity.GetTags() & NO_RESPONSE) != (other.GetTags() & NO_RESPONSE)) || other.GetComponent<comp::House>())
			{
				audio.shouldBroadcast = true;
				audio.is3D = true;
				audio.type = ESoundEvent::Player_OnRangeAttackHit;
			}

			if (audioState)
			{
				audioState->data.emplace(audio);
			}

			if (sameTeam)
			{
				if (other.GetComponent<comp::House>())
				{
					thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
				}
				return; //these guys are on the same team
			}

			if ((entity.GetTags() & GOOD) && (other.GetTags() & DEFENCE)
				 || attackEntity.GetTags() & NO_RESPONSE && other.GetTags() & NO_RESPONSE)
			{
				return;
			}

			comp::RangeAttackAbility* attackAbility = entity.GetComponent<comp::RangeAttackAbility>();

			if (attackAbility)
			{
				comp::Health* otherHealth = other.GetComponent<comp::Health>();
				if (otherHealth && !sameTeam)
				{
					otherHealth->currentHealth -= attackAbility->attackDamage;
					// update Health on network
					scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);

					// Blood particle
					if (other.GetComponent<comp::PARTICLEEMITTER>())
					{
						other.RemoveComponent<comp::PARTICLEEMITTER>();
					}
					other.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,6,0 }, 50, 5.f, PARTICLEMODE::BLOOD, 1.5f, 1.f, true);

					scene.publish<EComponentUpdated>(other, ecs::Component::PARTICLEMITTER);
				}

				//if (other.GetComponent<comp::Tag<TagType::DEFENCE>>())
				//{
				//	//TODO: add building particles
				//	if (other.GetComponent<comp::PARTICLEEMITTER>())
				//	{
				//		other.RemoveComponent<comp::PARTICLEEMITTER>();
				//	}
				//	other.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,10,0 }, 50, 10.f, PARTICLEMODE::SMOKEAREA, 3.5f, 1.f, true);

				//	scene.publish<EComponentUpdated>(other, ecs::Component::PARTICLEMITTER);
				//}

				//Change animation when taken damage
				comp::AnimationState* anim = other.GetComponent<comp::AnimationState>();
				if (anim)
				{
					anim->toSend = EAnimationType::TAKE_DAMAGE;
				}

				if (!other.GetComponent<comp::Tag<STATIC>>())
				{
					thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
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
			return;
		});
}

Entity CombatSystem::CreateAreaAttackCollider(HeadlessScene& scene, sm::Vector3 position, float size) 
{
	Entity e = scene.CreateEntity();
	e.AddComponent<comp::Transform>()->position = position;

	e.AddComponent<comp::SphereCollider>()->Radius = size;
	
	e.AddComponent<comp::Tag<TagType::DYNAMIC>>();
	e.AddComponent<comp::Tag<TagType::NO_RESPONSE>>();

	e.AddComponent<comp::SelfDestruct>()->lifeTime = 0.5f;
	e.AddComponent<comp::Network>();
	return e;
}

void CombatSystem::DoDamage(HeadlessScene& scene, Entity attacker, Entity attackCollider, Entity target, float damage, float knockback, AttackType type)
{
	bool doDamage = true;
	bool playHitSound = false;
	bool playTargetHitSound = true;
	bool doKnockback = true;

	// CHECKS
	// is caster already dead
	if (attacker.IsNull())
	{
		attackCollider.GetComponent<comp::SelfDestruct>()->lifeTime = 0.f;
		return;
	}
	
	// hit self
	if (target == attacker)
		return;

	bool isStaticTarget = target.HasComponent<comp::Tag<STATIC>>();
	bool isHouseTarget = target.HasComponent<comp::House>();
	bool isPlayerAttacker = attacker.HasComponent<comp::Player>();
	// hit map bounds
	if (isStaticTarget && !isHouseTarget)
	{
		return;
	}
	else if (isHouseTarget) // hit a house
	{
		if (isPlayerAttacker)
		{
			doDamage = false;
			playHitSound = true;
		}
		doKnockback = false;
		playTargetHitSound = false;
	}
	else if (isStaticTarget) // hit a collider
	{
		doDamage = false;
		doKnockback = false;
		playTargetHitSound = false;
		playHitSound = false;
	}

	// hit entity on same team
	tag_bits goodOrBad = TagType::GOOD | TagType::BAD;
	if ((attacker.GetTags() & goodOrBad) == (target.GetTags() & goodOrBad))
	{
		doDamage = false;
		playTargetHitSound = false;

		if (target.GetComponent<comp::Player>())
		{
			doKnockback = false;
		}
	}

	if (attacker.GetComponent<comp::Tag<GOOD>>() && target.GetComponent<comp::Tag<DEFENCE>>())
	{
		//good vs defense are on the same team aswell
		doDamage = false;
		playTargetHitSound = false;
		playHitSound = true;
	}

	

	// SOUND
	audio_t audio = {
		ESoundEvent::NONE,
		target.GetComponent<comp::Transform>()->position,
		1.f,
		100.f,
		false,
		false,
		false,
		false,
	};

	comp::AudioState* audioState = attacker.GetComponent<comp::AudioState>();
	if (!audioState)
		audioState = target.GetComponent<comp::AudioState>();
	if (audioState)
	{
		if (playTargetHitSound)
		{
			// Add some sound effects
			if (target.GetComponent<comp::Player>())
			{
				audio.type = ESoundEvent::Player_OnDmgRecieved;
			}
			else if (target.GetComponent<comp::NPC>())
			{
				audio.type = ESoundEvent::Enemy_OnDmgRecieved;	
				audio.is3D = true;
			}
		
			audioState->data.emplace(audio);
		}

		if (playHitSound)
		{
			if (type == AttackType::MELEE && attacker.GetComponent<comp::Player>())
			{
				audio.type = ESoundEvent::Player_OnMeleeAttackHit;
				audio.is3D = false;
				audio.shouldBroadcast = false;
				audioState->data.emplace(audio);
			}
		}
	}

	
	comp::Health* otherHealth = target.GetComponent<comp::Health>();
	if (otherHealth && doDamage)
	{
		otherHealth->currentHealth -= damage;
		// update Health on network
		scene.publish<EComponentUpdated>(target, ecs::Component::HEALTH);

		if (target.GetComponent<comp::PARTICLEEMITTER>())
		{
			target.RemoveComponent<comp::PARTICLEEMITTER>();
		}

		if (target.GetComponent<comp::Tag<TagType::DEFENCE>>())
		{
			//Smoke particles
			target.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,10,0 }, 50, 10.f, PARTICLEMODE::SMOKEAREA, 3.5f, 1.f, true);
		}
		else
		{
			// Blood particle
			target.AddComponent<comp::PARTICLEEMITTER>(sm::Vector3{ 0,6,0 }, 50, 5.f, PARTICLEMODE::BLOOD, 1.5f, 1.f, true);
		}

		scene.publish<EComponentUpdated>(target, ecs::Component::PARTICLEMITTER);
	}

	sm::Vector3 knockbackDir;
	comp::Velocity* attackVel = attackCollider.GetComponent<comp::Velocity>();
	if (attackVel)
	{
		knockbackDir = attackVel->vel;
		knockbackDir.Normalize();
	}
	else
	{
		knockbackDir = target.GetComponent<comp::Transform>()->position - attacker.GetComponent<comp::Transform>()->position;
		knockbackDir.Normalize();
	}


	if (doKnockback)
	{
		comp::TemporaryPhysics* p = target.AddComponent<comp::TemporaryPhysics>();

		comp::TemporaryPhysics::Force force = {};
		knockbackDir.y = 0.5f;
		knockbackDir.Normalize();

		force.force = knockbackDir * knockback;
		force.isImpulse = true;
		force.drag = 0.0f;
		force.actingTime = 0.2f;

		p->forces.push_back(force);

		auto gravity = ecs::GetGravityForce();
		p->forces.push_back(gravity);
	}
	
}

