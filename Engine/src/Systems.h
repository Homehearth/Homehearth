#pragma once
#include "Scene.h"
#include "CollisionSystem.h"
#include "PathFinderManager.h"
#include "QuadTree.h"

namespace Systems
{
	// abilities
	void UpdateAbilities(HeadlessScene& scene, float dt);
	void HealingSystem(HeadlessScene& scene, float dt);
	void HeroLeapSystem(HeadlessScene& scene, float dt);

	void HealthSystem(HeadlessScene& scene, float dt, Currency& money_ref);
	void SelfDestructSystem(HeadlessScene& scene, float dt);

	void MovementSystem(HeadlessScene& scene, float dt);
	void MovementColliderSystem(HeadlessScene& scene, float dt);
	void LightSystem(Scene& scene, float dt);

	void TransformAnimationSystem(HeadlessScene& scene, float dt);

	void UpdateDynamicQT(HeadlessScene& scene, QuadTree* qtDynamic);
	void FetchCollidingList(HeadlessScene& scene, QuadTree* qt, QuadTree* qtDynamic);
	void ClearCollidingList(HeadlessScene& scene, QuadTree* qtDynamic);

	void CheckCollisions(HeadlessScene& scene, float dt);

	void AISystem(HeadlessScene& scene, PathFinderManager* aiHandler);
}
