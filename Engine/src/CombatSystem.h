#pragma once
class CombatSystem
{
public:
	//Updates all systems related to combat
	static void UpdateCombatSystem(HeadlessScene& scene, float dt);

private:
	//Disallow creating an instance of this object...
	CombatSystem() = default;

	//Update all entity with melee abilities
	static void UpdateMelee(HeadlessScene& scene);
	//Update all entity with range abilities
	static void UpdateRange(HeadlessScene& scene);
	//Update target point if entity is player
	static void UpdateTargetPoint(Entity entity, sm::Vector3* targetPoint);
	//Creates an melee attack entity that holds multiple components
	static Entity CreateAttackEntity(Entity entity, HeadlessScene& scene, comp::Transform* transform, comp::MeleeAttackAbility* stats);
	//Creates an ranged attack entity that holds multiple components
	static Entity CreateAttackEntity(Entity entity, HeadlessScene& scene, comp::Transform* transform, comp::RangeAttackAbility* stats);
	//Creates behavior for collision with an attack entity
	static void AddCollisionMeleeBehavior(Entity entity, Entity attackEntity, HeadlessScene& scene);
	static void AddCollisionRangeBehavior(Entity entity, Entity attackEntity, HeadlessScene& scene);
};

