#include "EnginePCH.h"
#include "Systems.h"

// TODO:
//#include "Tags.h"
const unsigned char GOOD = 4;
const unsigned char BAD = 8;

void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each Good Guy.
	scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<GOOD>>([&](Entity& player, comp::Attack& playerAttack, comp::Health& playerHealth, comp::Transform& playerTransform, comp::Tag<GOOD>&)
		{
			// For Each Bad Guy.
			scene.ForEachComponent<comp::Attack, comp::Health, comp::Transform, comp::Tag<BAD>>([&](Entity& enemy, comp::Attack& enemyAttack, comp::Health& enemyHealth, comp::Transform& enemyTransform, comp::Tag<BAD>&)
				{

					//TODO: Change it from being sphere range check to be OBB hit detection
					
					//CQC
					if (!playerAttack.isRanged || !enemyAttack.isRanged)
					{
						// Calculate the distance between player and enemy.
						const auto dist = (playerTransform.position - enemyTransform.position).Length();
					
						//Decreases cooldown between attacks
						if (playerAttack.cooldownTimer > 0.f)
							playerAttack.cooldownTimer -= 1.f * dt;
						if (enemyAttack.cooldownTimer > 0.f)
							enemyAttack.cooldownTimer -= 1.f * dt;

						// Check if attack is in range.
						if (playerAttack.attackRange >= dist && playerAttack.cooldownTimer <= 0.f && enemyHealth.isAlive)
						{

							// Perform battle logic.
							if ((playerAttack.isAttacking & playerHealth.isAlive) == TRUE)
							{
								dx::BoundingOrientedBox attackOBB;
								attackOBB.Center = playerTransform.position + sm::Vector3(0.f, 0.f, 1.f);
								if (attackOBB.Contains(enemyTransform.position) != dx::ContainmentType::DISJOINT)
									LOG_INFO("OBB intersects with enemy");
								playerAttack.cooldownTimer = playerAttack.attackSpeed; //Set attacker on cooldown
								enemyHealth.currentHealth -= playerAttack.attackDamage; //Decrease health by the attackdamage
								LOG_INFO("Player %u: Attack landed.", player.GetComponent<comp::Network>()->id);
							}
						}

						// Check if attack is in range.
						if (enemyAttack.attackRange >= dist && enemyAttack.cooldownTimer <= 0.f && playerHealth.isAlive)
						{

							// Perform battle logic.
							if ((enemyAttack.isAttacking & enemyHealth.isAlive) == TRUE)
							{
								enemyAttack.cooldownTimer = enemyAttack.attackSpeed; //Set attacker on cooldown
								playerHealth.currentHealth -= enemyAttack.attackDamage; //Decrease health by the attackdamage
								LOG_INFO("Enemy attack landed.");
							}
						}
					}

					//Ranged Combat
					else
					{
						//Spawn a projectile
						//Check if it hits
						//Do damage calcs
						//Delete projectile when hit?
					}

					//Reset to non attacking state
					if (playerAttack.isAttacking)
						playerAttack.isAttacking = false;
					if (enemyAttack.isAttacking)
						enemyAttack.isAttacking = false;
				});
		});


	//Health System
	scene.ForEachComponent<comp::Health>([&](Entity& Entity, comp::Health& Health)
		{
			//Check if something should be dead, and if so set isAlive to false
			if (Health.currentHealth <= 0)
			{
				LOG_INFO("Entity died");
				Health.isAlive = false;
				Entity.Destroy(); //Don't really know if this should be called here
			}
		});
}

void Systems::MovementSystem(HeadlessScene& scene, float dt)
{
	//Transform
	scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt](comp::Transform& transform, comp::Velocity& velocity)
		{
			transform.position += velocity.vel * dt;
		});
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	//BoundingOrientedBox
	scene.ForEachComponent<comp::Transform, comp::BoundingOrientedBox>([&, dt](comp::Transform& transform, comp::BoundingOrientedBox& obb)
		{
			obb.Center = transform.position;
		});

	//BoundingSphere
	scene.ForEachComponent<comp::Transform, comp::BoundingSphere>([&, dt](comp::Transform& transform, comp::BoundingSphere& sphere)
		{
			sphere.Center = transform.position;
		});
}

void Systems::LightSystem(Scene& scene, float dt)
{
	//If you update the lightData update the info to the GPU
	scene.ForEachComponent<comp::Light>([&](comp::Light light)
		{
			scene.GetLights()->EditLight(light.lightData, light.index);
		});

	//If an Entity has both a Light and Transform component use Transform for position
	scene.ForEachComponent<comp::Transform, comp::Light>([&, dt](comp::Transform& transform, comp::Light& light)
		{
			light.lightData.position = sm::Vector4(transform.position.x, transform.position.y, transform.position.z, 1.f);
			scene.GetLights()->EditLight(light.lightData, light.index);
		});
	
}
