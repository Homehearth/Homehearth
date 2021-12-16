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

void CombatSystem::UpdateTeleport(HeadlessScene& scene, Blackboard* blackboard)
{
	scene.ForEachComponent<comp::BlinkAbility, comp::Transform>([&](Entity entity, comp::BlinkAbility& teleportAbility, comp::Transform& transform)
		{
			PathFinderManager* pathFinderManager = blackboard->GetPathFindManager();

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
							entity.GetComponent<comp::Transform>()->position = newPos;

							if (entity.GetComponent<comp::ParticleEmitter>())
								entity.RemoveComponent<comp::ParticleEmitter>();

							sm::Vector3 dirNormal = dir;
							dirNormal.Normalize();
							entity.AddComponent<comp::ParticleEmitter>(sm::Vector3(0, 6, 0), 200, 3.0f, ParticleMode::MAGEBLINK, 0.5f, -10.f, TRUE);
							entity.GetComponent<comp::ParticleEmitter>()->direction = dirNormal;
							scene.publish<EComponentUpdated>(entity, ecs::Component::PARTICLEMITTER);

							dirNormal *= -1;
							Entity blinkParticle = scene.CreateEntity();
							blinkParticle.AddComponent<comp::Transform>(newPos);
							blinkParticle.AddComponent<comp::ParticleEmitter>(sm::Vector3(0, 6, 0), 200, 3.0f, ParticleMode::MAGEBLINK, 0.5f, 10.f, TRUE);
							blinkParticle.GetComponent<comp::ParticleEmitter>()->direction = dirNormal;
							blinkParticle.AddComponent<comp::Network>();
							blinkParticle.UpdateNetwork();

							hasSetTarget = true;
							entity.UpdateNetwork();
							if (pathFinderManager->ReverseAStar(entity.GetComponent<comp::Transform>()->position))
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

					Entity collider = CreateAreaAttackCollider(scene, entity.GetComponent<comp::Transform>()->position, 10, dashAbility.useTime);
					collider.AddComponent<comp::Velocity>()->vel = dashAbility.velocityBeforeDash * dashAbility.force;

					//entity.AddComponent<comp::ParticleEmitter>(sm::Vector3(0, 6, 0), 100, 10.f, ParticleMode::MAGEBLINK, 2.f, 1.f, FALSE);

					CollisionSystem::Get().AddOnCollisionEnter(collider, [=](Entity thisEntity, Entity other)
						{
							tag_bits goodOrBad = TagType::GOOD | TagType::BAD;
							if ((thisEntity.GetTags() & goodOrBad) == (other.GetTags() & goodOrBad))
							{
								return;
							}
							if (other.GetComponent<comp::Tag<DYNAMIC>>() && other != entity)
							{
								sm::Vector3 knockbackDir = dir.Cross(sm::Vector3::Up);
								sm::Vector3 toTarget = other.GetComponent<comp::Transform>()->position - entity.GetComponent<comp::Transform>()->position;
								if (toTarget.Dot(knockbackDir) < 0)
									knockbackDir *= -1;
								knockbackDir.y = 1.f;
								knockbackDir.Normalize();
								AddKnockback(other, knockbackDir, 50.0f);
							}
						});

				}
			}

			if (ecs::IsUsing(&dashAbility))
			{
				entity.GetComponent<comp::Velocity>()->vel = dashAbility.velocityBeforeDash * dashAbility.force;

			}


		});
}

void CombatSystem::UpdateBlock(HeadlessScene& scene, float dt)
{
	scene.ForEachComponent<comp::ShieldBlockAbility>([&](Entity entity, comp::ShieldBlockAbility& ability)
		{
			sm::Vector3* updateTargetPoint = nullptr;

			UpdateTargetPoint(entity, updateTargetPoint);

			if (ecs::ReadyToUse(&ability, updateTargetPoint))
			{
				ability.isCooldownActive = false;
			}

			if (ecs::IsUsing(&ability))
			{
				ability.cooldownTimer = (ability.damageTaken / ability.maxDamage) * ability.cooldown;
			}
			else
			{
				if (!ability.isCooldownActive)
				{
					ability.timeSinceUse += dt;
					if (ability.timeSinceUse > 5.0f)
					{
						ability.isCooldownActive = true;
						ability.damageTaken = 0.0f;
						ability.timeSinceUse = 0.0f;
					}
				}
			}

		});
}

void CombatSystem::UpdateCombatSystem(HeadlessScene& scene, float dt, Blackboard* blackboard)
{
	PROFILE_FUNCTION();

	// For Each entity that can use melee attack.
	UpdateMelee(scene);

	//For each entity that can use range attack
	UpdateRange(scene);

	//For each entity that can use teleport
	UpdateTeleport(scene, blackboard);

	//For each entity that can use Dash
	UpdateDash(scene);

	UpdateBlock(scene, dt);
}

void CombatSystem::UpdateEffects(HeadlessScene& scene, float dt)
{
	scene.ForEachComponent<comp::Velocity, comp::Stun>([=](Entity e, comp::Velocity& v, comp::Stun& s)
		{
			v.vel *= s.movementSpeedAlt;

			s.stunTime -= dt;
			if (s.stunTime <= 0.0f)
				e.RemoveComponent<comp::Stun>();
		});
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

	if (entity.GetComponent<comp::NPC>())
		attackEntity.AddComponent<comp::Tag<TagType::ENEMY_ATTACK>>();
	else
		attackEntity.AddComponent<comp::Tag<TagType::PLAYER_ATTACK>>();

	comp::SphereCollider* bos = attackEntity.AddComponent<comp::SphereCollider>();

	bos->Radius = stats->attackRange;
	float attackRangeMultiplier = 2.f;

	attackEntity.AddComponent<comp::PlayerReference>()->player = entity;

	sm::Vector3 targetDir = stats->targetPoint - transform->position;
	targetDir.Normalize();
	t->position = transform->position + targetDir * stats->attackRange * attackRangeMultiplier;
	t->position.y = bos->Radius;
	t->rotation = transform->rotation;

	comp::SelfDestruct* selfDestruct = attackEntity.AddComponent<comp::SelfDestruct>();
	selfDestruct->lifeTime = stats->lifetime;

	//#if RENDER_COLLIDERS
	attackEntity.AddComponent<comp::Network>();
	//#endif

	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [=, &scene](Entity thisEntity, Entity other)
		{
			comp::MeleeAttackAbility* ability = nullptr;
			if (!entity.IsNull())
				ability = entity.GetComponent<comp::MeleeAttackAbility>();

			if (ability)
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
	attackEntity.AddComponent<comp::Tag<DYNAMIC>>();
	attackEntity.AddComponent<comp::Tag<NO_RESPONSE>>();
	attackEntity.AddComponent<comp::Tag<RANGED_ATTACK>>();

	if(entity.GetComponent<comp::NPC>())
		attackEntity.AddComponent<comp::Tag<TagType::ENEMY_ATTACK>>();
	else
		attackEntity.AddComponent<comp::Tag<TagType::PLAYER_ATTACK>>();

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

		// This is the collider and this entity wont be sent over network
		comp::RangeAttackAbility* ability = entity.GetComponent<comp::RangeAttackAbility>();
		Entity explosionCollider = CreateAreaAttackCollider(scene, attackEntity.GetComponent<comp::Transform>()->position, ability->attackRange, 0.1f);

		// Only this part will be sent over network
		Entity explosionEffect = scene.CreateEntity();
		explosionEffect.AddComponent<comp::Network>();
		explosionEffect.AddComponent<comp::Transform>()->position = attackEntity.GetComponent<comp::Transform>()->position;
		explosionEffect.AddComponent<comp::SelfDestruct>()->lifeTime = 0.5f;
		explosionEffect.AddComponent<comp::ParticleEmitter>(sm::Vector3{ 0,0,0 }, 200, 6.f, ParticleMode::EXPLOSION, 2.0f, 40.f, false);


		CollisionSystem::Get().AddOnCollisionEnter(explosionCollider, [=, &scene](Entity expl, Entity other)
			{
				comp::RangeAttackAbility* ability = nullptr;
				if (!entity.IsNull())
					ability = entity.GetComponent<comp::RangeAttackAbility>();
				if (ability)
					DoDamage(scene, entity, expl, other, ability->attackDamage, 40.0f, AttackType::RANGE);
			});
	};
	
	attackEntity.AddComponent<comp::ParticleEmitter>(sm::Vector3{ 0,0,0 }, 200, 1.f, ParticleMode::MAGERANGE, 1.7f, 1.f, FALSE);

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

	CollisionSystem::Get().AddOnCollisionEnter(attackEntity, [=, &scene](Entity thisEntity, Entity other)
		{
			if (other.HasComponent<comp::Tag<STATIC>>() && !other.HasComponent<comp::House>())
				return;

			if (thisEntity.GetTags() & RANGED_ATTACK && other.GetTags() & RANGED_ATTACK)
			{
				return;
			}

			thisEntity.GetComponent<comp::SelfDestruct>()->lifeTime = 0.0f;
		});


	return attackEntity;
}

Entity CombatSystem::CreateAreaAttackCollider(HeadlessScene& scene, sm::Vector3 position, float size, float lifeTime)
{
	Entity e = scene.CreateEntity();
	e.AddComponent<comp::Transform>()->position = position;

	e.AddComponent<comp::SphereCollider>()->Radius = size;

	e.AddComponent<comp::Tag<TagType::DYNAMIC>>();
	e.AddComponent<comp::Tag<TagType::NO_RESPONSE>>();

	e.AddComponent<comp::SelfDestruct>()->lifeTime = lifeTime;

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

	if (target.HasComponent<comp::Invincible>())
	{
		doDamage = false;
		playTargetHitSound = false;
		playHitSound = true;
		doKnockback = false;
	}

	

	bool isStaticTarget = target.HasComponent<comp::Tag<STATIC>>();
	bool isHouseTarget = target.HasComponent<comp::House>();
	bool isDefenseTarget = target.HasComponent<comp::Tag<DEFENCE>>();
	bool isPlayerAttacker = attacker.HasComponent<comp::Player>();
	// hit map bounds
	if (isStaticTarget && !isHouseTarget && !isDefenseTarget)
	{
		return;
	}
	else if (isHouseTarget) // hit a house
	{
		playHitSound = false;
		if (isPlayerAttacker)
		{
			doDamage = false;
			playHitSound = true;
		}
		doKnockback = false;
		playTargetHitSound = false;
	}
	else if (isStaticTarget && !isDefenseTarget) // hit a collider
	{
		doDamage = false;
		doKnockback = false;
		playTargetHitSound = false;
		playHitSound = false;
	}

	comp::ShieldBlockAbility* block = target.GetComponent<comp::ShieldBlockAbility>();
	if (block)
	{
		if (ecs::IsUsing(block))
		{
			sm::Vector3 toAttacker = attacker.GetComponent<comp::Transform>()->position - target.GetComponent<comp::Transform>()->position;
			toAttacker.Normalize();
			sm::Vector3 targetForward = ecs::GetForward(*target.GetComponent<comp::Transform>());
			//Check if in front
			if (toAttacker.Dot(targetForward) > 0)
			{
				doDamage = false;
				playTargetHitSound = false;
				playHitSound = true;
				doKnockback = true;
				knockback *= 0.5f;
				block->damageTaken += damage;

				if (block->damageTaken >= block->maxDamage)
				{
					block->cooldownTimer = block->cooldown;
					ecs::CancelAbility(block);
					block->damageTaken = 0.0f;
					block->isCooldownActive = true;

				}
			}
			else {
				ecs::CancelAbility(block);
				block->damageTaken = 0.0f;
				block->isCooldownActive = true;
			}
		}
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
		else if (target.GetComponent<comp::NPC>())
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
			else if (target.GetComponent<comp::Villager>())
			{
				audio.type = ESoundEvent::Player_OnDmgRecieved;
				audio.is3D = true;
				audio.shouldBroadcast = true;
			}

			audioState->data.emplace(audio);
		}

		if (playHitSound)
		{
			if (type == AttackType::MELEE)
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

		if (target.GetComponent<comp::ParticleEmitter>())
		{
			target.RemoveComponent<comp::ParticleEmitter>();
		}
		
		if (target.GetComponent<comp::Tag<TagType::DEFENCE>>())
		{
			//Smoke particles
			target.AddComponent<comp::ParticleEmitter>(sm::Vector3{ 0,10,0 }, 50, 10.f, ParticleMode::SMOKEAREA, 3.5f, 1.f, TRUE);
		}
		else
		{
			// Blood particle
			target.AddComponent<comp::ParticleEmitter>(sm::Vector3{ 0,6,0 }, 50, 1.5f, ParticleMode::BLOOD, 2.0f, 1.f, TRUE);
		}

		if (otherHealth->currentHealth <= 0.0f)
		{
			comp::KillDeaths* kd = attacker.GetComponent<comp::KillDeaths>();
			if (kd)
			{
				kd->kills++;
				scene.publish<EComponentUpdated>(attacker, ecs::Component::KD);
			}
		}

		scene.publish<EComponentUpdated>(target, ecs::Component::PARTICLEMITTER);

		comp::AnimationState* anim = target.GetComponent<comp::AnimationState>();
		if (anim)
		{
			anim->toSend = EAnimationType::TAKE_DAMAGE;
		}
	}

	sm::Vector3 knockbackDir;
	if (type == AttackType::RANGE)
	{
		knockbackDir = target.GetComponent<comp::Transform>()->position - attackCollider.GetComponent<comp::Transform>()->position;
		knockbackDir.Normalize();
		knockbackDir.y = 1.0f;
		knockbackDir.Normalize();
	}
	else
	{
		knockbackDir = target.GetComponent<comp::Transform>()->position - attacker.GetComponent<comp::Transform>()->position;
		knockbackDir.Normalize();
		knockbackDir.y = 1.0f;
		knockbackDir.Normalize();
	}

	if (doKnockback)
	{
		AddKnockback(target, knockbackDir, knockback);
	}
}

void CombatSystem::AddKnockback(Entity target, sm::Vector3 dir, float power)
{
	comp::TemporaryPhysics* p = target.AddComponent<comp::TemporaryPhysics>();

	comp::TemporaryPhysics::Force force = {};

	force.force = dir * power;
	force.isImpulse = true;
	force.drag = 0.0f;
	force.actingTime = 0.2f;

	p->forces.push_back(force);

	auto gravity = ecs::GetGravityForce();
	p->forces.push_back(gravity);
}

