#include "EnginePCH.h"
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
				float decreaseValue = 0.75f;
				if (targetPoint)
				{
					sm::Vector3 direction = *targetPoint - transform.position;
					direction.Normalize();
					direction *= teleportAbility.distance;

					bool hasSetTarget = false;
					while (!hasSetTarget && direction.Length() > 7.0f)
					{
						sm::Vector3 newPos = transform.position + direction;
						if (pathFinderManager->FindClosestNode(newPos)->reachable)
						{
							entity.GetComponent<comp::Transform>()->position = transform.position + direction;
							hasSetTarget = true;
						}
						else
						{
							//Lower length of vector by 25% every failed try
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
			}
			if(ecs::IsUsing(&dashAbility))
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

	sm::Vector3 targetDir = stats->targetPoint - transform->position;
	targetDir.Normalize();
	t->position = transform->position + targetDir * stats->attackRange * 0.5f + sm::Vector3(0, 4, 0);
	t->rotation = transform->rotation;

	bos->Center = t->position;

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

			if (otherHealth && attackAbility)
			{
				otherHealth->currentHealth -= attackAbility->attackDamage;
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
				// update Health on network
				scene.publish<EComponentUpdated>(other, ecs::Component::HEALTH);

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
