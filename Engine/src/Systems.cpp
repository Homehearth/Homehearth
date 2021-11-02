#include "EnginePCH.h"
#include "Systems.h"


void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each Good Guy (Player).
	scene.ForEachComponent<comp::CombatStats, comp::Health, comp::Transform, comp::Player>([&](Entity& player, comp::CombatStats& playerStats, comp::Health& playerHealth, comp::Transform& playerTransform, comp::Player&)
	{
		if (playerHealth.isAlive)
		{
			// Decreases cooldown between attacks.
			if (playerStats.cooldownTimer > 0.f)
				playerStats.cooldownTimer -= 1.f * dt;

			// For Each Bad Guy (Enemy).
			scene.ForEachComponent<comp::CombatStats, comp::Health, comp::Transform, comp::NPC>([&](Entity& enemy, comp::CombatStats& enemyStats, comp::Health& enemyHealth, comp::Transform& enemyTransform, comp::NPC&)
			{
				if (enemyHealth.isAlive)
				{
					// Decreases cooldown between attacks
					if (enemyStats.cooldownTimer > 0.f)
						enemyStats.cooldownTimer -= 1.f * dt;

					//
					// PLAYER LOGIC
					//
					if (playerStats.isAttacking)
					{
						//Creates an entity that's used to check collision if an attack lands.
						Entity attackCollider = scene.CreateEntity();
						attackCollider.AddComponent<comp::Transform>()->position = playerTransform.position + ecs::GetForward(playerTransform) * -1;
						attackCollider.AddComponent<comp::BoundingOrientedBox>()->Center = playerTransform.position + ecs::GetForward(playerTransform) * -1;
						attackCollider.AddComponent<comp::Attack>()->lifeTime = playerStats.attackLifeTime;
						attackCollider.GetComponent<comp::Attack>()->damage = playerStats.attackDamage;

						LOG_INFO("Attack Collider Created!");

						//If the attack is ranged add a velocity to the entity.
						if (playerStats.isRanged)
						{
							sm::Vector3 vel = ecs::GetForward(playerTransform) * -10.f; //CHANGE HERE WHEN FORWARD GETS FIXED!!!!!!
							attackCollider.AddComponent<comp::Velocity>()->vel = vel;
						}

						CollisionSystem::Get().AddOnCollision(attackCollider, [&, attackCollider](Entity& enemyCol)
						{
							comp::Health* enemyHealth = enemyCol.GetComponent<comp::Health>();
							comp::NPC* enemyTag = enemyCol.GetComponent<comp::NPC>();
							comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

							if (enemyHealth && atk && enemyTag)
							{
								enemyHealth->currentHealth -= atk->damage;
								LOG_INFO("ATTACK COLLIDER HIT BAD GUY!");
								atk->lifeTime = 0.f;
							}
						});

						playerStats.cooldownTimer = playerStats.attackSpeed;
						playerStats.isAttacking = false;
					}


					//
					// ENEMY LOGIC
					//
					if (enemyStats.isAttacking)
					{
						//Creates an entity that's used to check collision if an attack lands
						Entity attackCollider = scene.CreateEntity();
						attackCollider.AddComponent<comp::Transform>()->position = enemyTransform.position + ecs::GetForward(enemyTransform);
						attackCollider.AddComponent<comp::BoundingOrientedBox>()->Center = enemyTransform.position + ecs::GetForward(enemyTransform);
						attackCollider.AddComponent<comp::Attack>()->lifeTime = enemyStats.attackLifeTime;
						attackCollider.GetComponent<comp::Attack>()->damage = enemyStats.attackDamage;

						LOG_INFO("Attack Collider Created!");

						//If the attack is ranged add a velocity to the entity.
						if (enemyStats.isRanged)
						{
							sm::Vector3 vel = ecs::GetForward(enemyTransform) * -10.f; //CHANGE HERE WHEN FORWARD GETS FIXED!!!!!!
							attackCollider.AddComponent<comp::Velocity>()->vel = vel;
						}

						CollisionSystem::Get().AddOnCollision(attackCollider, [&, attackCollider](Entity& playerCol)
						{
							comp::Health* playerHealth = playerCol.GetComponent<comp::Health>();
							comp::Player* playerTag = playerCol.GetComponent<comp::Player>();
							comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

							if (playerHealth && atk && playerTag)
							{
								playerHealth->currentHealth -= atk->damage;
								LOG_INFO("ATTACK COLLIDER HIT GOOD GUY!");
								atk->lifeTime = 0.f;
							}
						});

						enemyStats.cooldownTimer = enemyStats.attackSpeed;
						enemyStats.isAttacking = false;
					}
				}

			});
		}
	});


	//Health System
	scene.ForEachComponent<comp::Health>([&](Entity& Entity, comp::Health& Health)
	{
		//Check if something should be dead, and if so set isAlive to false
		if (Health.currentHealth <= 0)
		{
			LOG_INFO("Entity died");
			Health.isAlive = false;
			Entity.Destroy();
		}
	});

	//Projectile Life System
	scene.ForEachComponent<comp::Attack>([&](Entity& ent, comp::Attack& Projectile)
	{
		Projectile.lifeTime -= 1.f * dt;

		if (Projectile.lifeTime <= 0)
		{
			LOG_INFO("Attack Collider Destroyed");
			ent.Destroy();
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
void Systems::AISystem(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::NPC>([&](Entity entity, comp::NPC& npc)
	{
		comp::Transform* transformNPC = entity.GetComponent<comp::Transform>();
		Entity* currentClosestPlayer = nullptr;
		if (npc.currentNode)
		{
			if (sm::Vector3::Distance(npc.currentNode->position, transformNPC->position) <= npc.attackRange)
			{
				npc.state = comp::NPC::State::ATTACK;
			}
			else if ((sm::Vector3::Distance(npc.currentNode->position, transformNPC->position) >= npc.attackRange + 10 && npc.state == comp::NPC::State::ATTACK))
			{
				npc.state = comp::NPC::State::CHASE;
			}
		}
		switch (npc.state)
		{
		case comp::NPC::State::ATTACK:
			//Do attacking things
			break;
		case comp::NPC::State::CHASE:
			if (!npc.currentNode)
			{
				if (!npc.path.empty())
				{
					npc.currentNode = npc.path.at(0);
					npc.path.erase(npc.path.begin());
				}
				else
				{
					AIAStarSearch(entity, scene);
				}
			}
			else
			{

			}
			break;
		case comp::NPC::State::IDLE:
			break;
		}

		Entity* closestPlayer = FindClosestPlayer(scene, transformNPC->position);
		comp::Velocity* velocityTowardsPlayer = entity.GetComponent<comp::Velocity>();
		comp::Transform* transformCurrentClosestPlayer = closestPlayer->GetComponent<comp::Transform>();
		if (velocityTowardsPlayer)
		{
			velocityTowardsPlayer->vel = transformCurrentClosestPlayer->position - transformNPC->position;
			velocityTowardsPlayer->vel.Normalize();
			velocityTowardsPlayer->vel *= npc.movementSpeed;

		}


	});
}
Entity* FindClosestPlayer(HeadlessScene& scene, sm::Vector3 position)
{
	Entity* currentClosest = nullptr;
	comp::Transform* transformCurrentClosest = nullptr;

	scene.ForEachComponent < comp::Player>([&](Entity playerEntity, comp::Player& player)
	{
		if (currentClosest)
		{
			comp::Transform* transformPlayer = playerEntity.GetComponent<comp::Transform>();
			if (sm::Vector3::Distance(transformPlayer->position, position) < sm::Vector3::Distance(transformCurrentClosest->position, position))
			{
				LOG_INFO("Switching player");
				currentClosest = &playerEntity;
				transformCurrentClosest = currentClosest->GetComponent<comp::Transform>();
			}
		}
		else
		{
			currentClosest = &playerEntity;
			transformCurrentClosest = currentClosest->GetComponent<comp::Transform>();
		}
	});
	return currentClosest;
}
bool AIAStarSearch(Entity& npc, HeadlessScene& scene)
{
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();
	comp::Node* currentNode = npcComp->currentNode;

	std::vector<comp::Node*> closedList, openList;
	npcComp->path.clear();
	comp::Node* startingNode = currentNode, * goalNode = currentNode;
	openList.push_back(startingNode);
	startingNode->f = 0.f;
	startingNode->g = 0.f;
	startingNode->h = 0.f;
	startingNode->parent = startingNode;

	while (goalNode == currentNode)
	{
		//goalNode = New target node;
	}

	comp::Node* nodeToAdd = nullptr;
	int index = 0;
	while (!openList.empty() && nodeToAdd != goalNode)
	{
		nodeToAdd = openList.at(0);
		int indexToPop = 0;
		bool stop = false;
		for (unsigned int i = 0; i < openList.size(); i++)
		{
			if (openList.at(i)->f < nodeToAdd->f)
			{
				nodeToAdd = openList.at(i);
				indexToPop = i;
			}
		}
		openList.erase(openList.begin() + indexToPop);

		//Neighbors

		std::vector<comp::Node*> neighbors = nodeToAdd->connections;

		for (comp::Node* neighbor : neighbors)
		{
			if (neighbor->parent != nodeToAdd && neighbor != nodeToAdd)
			{
				if (!neighbor->parent)
				{
					neighbor->parent = nodeToAdd;
				}
				if (neighbor == goalNode)
				{
					nodeToAdd = goalNode;
					break;
				}
				if (neighbor->f == FLT_MAX)
				{
					float tempF = 0, tempG = 0, tempH = 0;

					tempG = nodeToAdd->g + (nodeToAdd->position - neighbor->position).Length();
					tempH = (goalNode->position - nodeToAdd->position).Length(); //Using euclidean distance
					tempF = tempG + tempH;
					neighbor->f = tempF;
					neighbor->g = tempG;
					neighbor->h = tempH;
				}
				stop = false;
				for (unsigned int i = 0; i < openList.size() && !stop; i++)
				{
					if (openList.at(i)->id == neighbor->id)
					{
						stop = true;
					}
				}
				if (closedList.size() > 0)
				{
					for (unsigned int i = 0; i < closedList.size() && !stop; i++)
					{
						if (closedList.at(i)->id == neighbor->id)
						{
							stop = true;
						}
					}
					if (!stop)
					{
						openList.push_back(neighbor);
					}
				}
				else
				{
					openList.push_back(neighbor);
				}
			}
		}

		closedList.push_back(nodeToAdd);

		index++;

	}

	//TracePath

	while (goalNode != startingNode)
	{
		//Insert currentNode to the path
		npcComp->path.insert(npcComp->path.begin(), currentNode);
		currentNode = currentNode->parent;
	}

	scene.ForEachComponent<comp::Node>([&](Entity entity, comp::Node& node)
	{
		node.ResetFGH();
		node.parent = nullptr;
	});

	return true;
}