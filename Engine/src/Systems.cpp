#include "EnginePCH.h"
#include "Systems.h"
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

comp::Node* FindClosestNode(HeadlessScene& scene, sm::Vector3 position)
{
	comp::Node* currentClosest = nullptr;

	scene.ForEachComponent<comp::Node>([&](comp::Node& node)
		{
			if (currentClosest)
			{
				if (sm::Vector3::Distance(node.position, position) < sm::Vector3::Distance(currentClosest->position, position))
				{
					currentClosest = &node;
				}
			}
			else
			{
				currentClosest = &node;
			}
		});

	return currentClosest;
}

bool Systems::AIAStarSearch(Entity& npc, HeadlessScene& scene)
{
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();
	comp::Transform* npcTransform = npc.GetComponent<comp::Transform>();
	comp::Node* currentNode = npcComp->currentNode;

	Entity* closestPlayer = FindClosestPlayer(scene, npcTransform->position);

	comp::Transform* playerTransform = closestPlayer->GetComponent<comp::Transform>();

	std::vector<comp::Node*> closedList, openList;
	npcComp->path.clear();
	comp::Node* startingNode = currentNode, * goalNode = FindClosestNode(scene, playerTransform->position);;
	openList.push_back(startingNode);
	startingNode->f = 0.f;
	startingNode->g = 0.f;
	startingNode->h = 0.f;
	startingNode->parent = startingNode;

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

void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	// For Each entity that can attack.
	scene.ForEachComponent<comp::CombatStats, comp::Transform>([&](Entity entity, comp::CombatStats& stats, comp::Transform& transform)
	{
		// Decreases cooldown between attacks.
		if (stats.cooldownTimer > 0.f)
			stats.cooldownTimer -= dt;

		//
		// attack LOGIC
		//
		if (stats.isAttacking && stats.cooldownTimer <= 0)
		{
			/*
			for (int i = 0; i < 100; i++)
			{
				Entity e = scene.CreateEntity();
				e.AddComponent<comp::Network>();
				e.AddComponent<comp::MeshName>("cube.obj");
				e.AddComponent<comp::Transform>()->position = transform.position + sm::Vector3(i) * 2;
			}
			*/
			//Creates an entity that's used to check collision if an attack lands.
			Entity attackCollider = scene.CreateEntity();
			attackCollider.AddComponent<comp::Transform>()->position = transform.position + stats.targetDir * 10.f;
			//attackCollider.AddComponent<comp::BoundingOrientedBox>()->Center = transform.position + stats.targetDir;
			attackCollider.AddComponent<comp::BoundingOrientedBox>()->Extents = sm::Vector3(2.f, 2.f, 2.f);
			comp::Attack* atk = attackCollider.AddComponent<comp::Attack>();
			atk->lifeTime = stats.attackLifeTime;
			atk->damage = stats.attackDamage;

			//If the attack is ranged add a velocity to the entity.
			if (stats.isRanged)
			{
				sm::Vector3 vel = stats.targetDir * 100.f; //CHANGE HERE WHEN FORWARD GETS FIXED!!!!!!
				attackCollider.AddComponent<comp::Velocity>()->vel = vel;
			}

			//DEBUG
			LOG_INFO("Attack Collider Created!");
			attackCollider.AddComponent<comp::Network>();
			//


			CollisionSystem::Get().AddOnCollision(attackCollider, [=](Entity other)
			{
				if (other == entity)
					return;


				comp::Health* otherHealth = other.GetComponent<comp::Health>();
				comp::Attack* atk = attackCollider.GetComponent<comp::Attack>();

				if (otherHealth)
				{
					otherHealth->currentHealth -= atk->damage;
					LOG_INFO("ATTACK COLLIDER HIT BAD GUY!");
					atk->lifeTime = 0.f;
				}
			});

			stats.cooldownTimer = stats.attackSpeed;
			stats.isAttacking = false;
		}

	});


	//Health System
	scene.ForEachComponent<comp::Health>([&](Entity& entity, comp::Health& health)
	{
		//Check if something should be dead, and if so set isAlive to false
		if (health.currentHealth <= 0)
		{
			LOG_INFO("Entity died");
			health.isAlive = false;
			if (!entity.GetComponent<comp::Player>())
			{
				entity.Destroy();
			}

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
		transform.previousPosition = transform.position;
		transform.position += velocity.vel * dt;
	});
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	//BoundingOrientedBox
	scene.ForEachComponent<comp::Transform, comp::BoundingOrientedBox>([&, dt](comp::Transform& transform, comp::BoundingOrientedBox& obb)
	{
		obb.Center = transform.position;
		obb.Orientation = transform.rotation;
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
		npc.currentNode = FindClosestNode(scene, transformNPC->position);

		Entity* closestPlayer = FindClosestPlayer(scene, transformNPC->position);
		comp::Velocity* velocityTowardsPlayer = entity.GetComponent<comp::Velocity>();
		comp::Transform* transformCurrentClosestPlayer = closestPlayer->GetComponent<comp::Transform>();
		if (npc.currentNode)
		{
			if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= npc.attackRange)
			{
				npc.state = comp::NPC::State::CHASE;
				LOG_INFO("Switching to CHASE State!");
			}
			else if ((sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) >= npc.attackRange + 100 && npc.state == comp::NPC::State::CHASE))
			{
				npc.state = comp::NPC::State::ASTAR;
				LOG_INFO("Switching to ASTAR State!");
			}
		}

		switch (npc.state)
		{
		case comp::NPC::State::CHASE:

			if (velocityTowardsPlayer)
			{
				velocityTowardsPlayer->vel = transformCurrentClosestPlayer->position - transformNPC->position;
				velocityTowardsPlayer->vel.Normalize();
				velocityTowardsPlayer->vel *= npc.movementSpeed;

			}
			break;
		case comp::NPC::State::ASTAR:
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
	});
}
