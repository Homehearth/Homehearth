#pragma once

class CombatSystem
{
public:
	enum class AttackType
	{
		MELEE,
		RANGE
	};

	//Updates all systems related to combat
	static void UpdateCombatSystem(HeadlessScene& scene, float dt);

private:
	//Disallow creating an instance of this object...
	CombatSystem() = default;

	//Update all entity with melee abilities
	static void UpdateMelee(HeadlessScene& scene);
	//Update all entity with range abilities
	static void UpdateRange(HeadlessScene& scene);
	//Update all entity with range abilities
	static void UpdateTeleport(HeadlessScene& scene);
	//Update all entity with dash abilities
	static void UpdateDash(HeadlessScene& scene);
	//Update target point if entity is player
	static void UpdateTargetPoint(Entity entity, sm::Vector3* targetPoint);
	//Creates an melee attack entity that holds multiple components
	static Entity CreateAttackEntity(Entity entity, HeadlessScene& scene, comp::Transform* transform, comp::MeleeAttackAbility* stats);
	//Creates an ranged attack entity that holds multiple components
	static Entity CreateAttackEntity(Entity entity, HeadlessScene& scene, comp::Transform* transform, comp::RangeAttackAbility* stats);

	static Entity CreateAreaAttackCollider(HeadlessScene& scene, sm::Vector3 position, float size, float lifetime);
	static void DoDamage(HeadlessScene& scene, Entity attacker, Entity attackCollider,  Entity target, float damage, float knockback, AttackType type);

	static void AddKnockback(Entity target, sm::Vector3 dir, float power);
};

