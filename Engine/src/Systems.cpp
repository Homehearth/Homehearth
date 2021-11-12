#include "EnginePCH.h"
#include "Systems.h"
#include "Text.h"
#include "Components.h"
#include "Healthbar.h"

Entity FindClosestPlayer(HeadlessScene& scene, sm::Vector3 position, comp::NPC* npc)
{
	
	comp::Transform* transformCurrentClosest = nullptr;

	scene.ForEachComponent < comp::Player>([&](Entity& playerEntity, comp::Player& player)
	{
		if (!npc->currentClosest.IsNull())
		{
			transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
			comp::Transform* transformPlayer = playerEntity.GetComponent<comp::Transform>();
			if (sm::Vector3::Distance(transformPlayer->position, position) < sm::Vector3::Distance(transformCurrentClosest->position, position))
			{
				LOG_INFO("Switching player");
				npc->currentClosest = playerEntity;
				transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
			}
		}
		else
		{
			npc->currentClosest = playerEntity;
			transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
		}
	});

	return npc->currentClosest;
}

bool ReachedNode(Entity* entity,comp::Node* node)
{
	comp::Transform* entityTransform = entity->GetComponent<comp::Transform>();
	if (sm::Vector3::Distance(entityTransform->position, node->position) < 10.f)
	{
		return true;
	}
	else
	{
		return false;
	}
}

comp::Node* FindClosestNode(HeadlessScene& scene, sm::Vector3 position)
{
	comp::Node* currentClosest = nullptr;

	scene.ForEachComponent<comp::Node>([&](comp::Node& node)
		{
			if (currentClosest)
			{
				if (sm::Vector3::Distance(node.position, position) < sm::Vector3::Distance(currentClosest->position, position) && node.reachable)
				{
					currentClosest = &node;
				}
			}
			else
			{
				if(node.reachable)
					currentClosest = &node;
			}
		});

	return currentClosest;
}

bool AIAStarSearch(Entity& npc, HeadlessScene& scene)
{
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();
	comp::Transform* npcTransform = npc.GetComponent<comp::Transform>();
	comp::Node* currentNode = npcComp->currentNode;

	Entity closestPlayer = FindClosestPlayer(scene, npcTransform->position, npcComp);
	comp::Transform* playerTransform = closestPlayer.GetComponent<comp::Transform>();

	std::vector<comp::Node*> closedList, openList;
	npcComp->path.clear();
	comp::Node* startingNode = currentNode, * goalNode = FindClosestNode(scene, playerTransform->position);;
	openList.push_back(startingNode);
	startingNode->f = 0.f;
	startingNode->g = 0.f;
	startingNode->h = 0.f;
	startingNode->parent = startingNode;

	comp::Node* nodeToAdd = nullptr;
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
	}

	//TracePath
	if(goalNode)
		currentNode = goalNode;

	while (currentNode != startingNode)
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

void Systems::UpdateAbilities(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();
	for (auto type : entt::resolve())
	{
		using namespace entt::literals;

		entt::id_type abilityType[] = { type.info().hash() };
		for (auto e : scene.GetRegistry()->runtime_view(std::cbegin(abilityType), std::cend(abilityType)))
		{
			Entity entity(*scene.GetRegistry(), e);
			auto instance = type.func("get"_hs).invoke({}, entity);
			comp::IAbility* ability = instance.try_cast<comp::IAbility>();
			if (ability)
			{
				// Decreases cooldown between attacks.
				if (ability->delayTimer > 0.f)
					ability->delayTimer -= dt;
					

				if (ability->cooldownTimer > 0.f)
					ability->cooldownTimer -= dt;
				else
					ability->isReady = true;
			}
		}
	}

}

void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();
	
	// For Each entity that can attack.
	scene.ForEachComponent<comp::AttackAbility, comp::Transform>([&](Entity entity, comp::AttackAbility& stats, comp::Transform& transform)
		{
			//
			// attack LOGIC
			//
			if (ecs::IsUsing(&stats))
			{
				//Creates an entity that's used to check collision if an attack lands.
				Entity attackCollider = scene.CreateEntity();
				comp::Transform* t = attackCollider.AddComponent<comp::Transform>();
				comp::BoundingOrientedBox* box = attackCollider.AddComponent<comp::BoundingOrientedBox>();
				
				box->Extents = sm::Vector3(stats.attackRange * 0.5f, 10, stats.attackRange * 0.5f);
				
				sm::Vector3 targetDir = stats.targetPoint - transform.position;
				targetDir.Normalize();
				t->position = transform.position + targetDir * stats.attackRange * 0.5f + sm::Vector3(0, 4, 0);
				t->rotation = transform.rotation;

				box->Center = t->position;
				box->Orientation = t->rotation;


				comp::SelfDestruct* selfDestruct = attackCollider.AddComponent<comp::SelfDestruct>();
				selfDestruct->lifeTime = stats.lifetime;
				
				//If the attack is ranged add a velocity to the entity.
				if (stats.isRanged)
				{
					sm::Vector3 vel = targetDir * stats.projectileSpeed;
					attackCollider.AddComponent<comp::Velocity>()->vel = vel;
					attackCollider.AddComponent<comp::MeshName>()->name = "Sphere.obj";
				}
				attackCollider.AddComponent<comp::Network>();

				
				CollisionSystem::Get().AddOnCollision(attackCollider, [entity, &scene](Entity thisEntity, Entity other)
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
						comp::AttackAbility* stats = entity.GetComponent<comp::AttackAbility>();

						if (otherHealth)
						{
							otherHealth->currentHealth -= stats->attackDamage;
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
								force.force *= stats->attackDamage;
								
								force.isImpulse = true;
								force.drag = 0.0f;
								force.actingTime = 0.7f;

								p->forces.push_back(force);

								auto gravity = ecs::GetGravityForce();
								p->forces.push_back(gravity);
							}
							
						}
					});

		}

	});


}

void Systems::HealingSystem(HeadlessScene& scene, float dt)
{
	// HealAbility system
	scene.ForEachComponent<comp::HealAbility>([](comp::HealAbility& ability)
		{
			if (ecs::IsUsing(&ability))
			{
				LOG_INFO("Used healing ability");
			}
		});

}

void Systems::HealthSystem(HeadlessScene& scene, float dt, uint32_t& money_ref)
{
	//Entity destoys self if health <= 0
	scene.ForEachComponent<comp::Health>([&](Entity& entity, comp::Health& health)
		{
			//Check if something should be dead, and if so set isAlive to false
			if (health.currentHealth <= 0)
			{
				comp::Network* net = entity.GetComponent<comp::Network>();
				health.isAlive = false;
				if (!entity.GetComponent<comp::Player>())
				{
					entity.Destroy();
					money_ref += 5;
				}

			}
		});
}

void Systems::SelfDestructSystem(HeadlessScene& scene, float dt)
{
	//Entity destroys self after set time
	scene.ForEachComponent<comp::SelfDestruct>([&](Entity& ent, comp::SelfDestruct& s)
		{
			s.lifeTime -= dt;
			if (s.lifeTime <= 0)
			{
				ent.Destroy();
			}
		});
}

void Systems::MovementSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	//Transform

	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::TemporaryPhysics > ([&](Entity e, comp::Transform& t, comp::Velocity& v, comp::TemporaryPhysics& p)
		{
			v.vel = v.oldVel; // ignore any changes made to velocity made this frame
			auto& it = p.forces.begin();
			while(it != p.forces.end())
			{
				comp::TemporaryPhysics::Force& f = *it;
				
				if (f.isImpulse)
				{
					if (f.wasApplied)
					{
						// Apply drag
						v.vel *= 1.0f - (dt * f.drag);
					}
					else
					{
						// Apply force once
						v.vel = f.force;
						f.wasApplied = true;
					}
				}
				else
				{
					// Apply constant force
					v.vel += f.force * dt * 2.f;
				}

				sm::Vector3 newPos = t.position + v.vel * dt;
				if (newPos.y < 0.0f)
				{
					v.vel.y = 0;
					f.force.y = 0;
				}

				if (f.force.Length() < 0.01f)
				{
					f.actingTime = 0.0f;
				}

				f.actingTime -= dt;
				if (f.actingTime <= 0.0f)
				{
					it = p.forces.erase(it);
					if (p.forces.size() == 0)
					{
						e.RemoveComponent<comp::TemporaryPhysics>();
						return;
					}
				}
				else
				{
					it++;
				}
			}
		});

	{
		PROFILE_SCOPE("Add Velocity to Transform");
		scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt]
		(Entity e, comp::Transform& transform, comp::Velocity& velocity)
			{

				if (velocity.vel.Length() > 0.01f)
				{
					e.UpdateNetwork();
				}
				
				transform.position += velocity.vel * dt;
				
				if (transform.position.y < 0.f)
				{
					transform.position.y = 0.f;
					velocity.vel.y = 0;
				}


				velocity.oldVel = velocity.vel; // updated old vel position
			});
	}
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	//BoundingOrientedBox
	scene.ForEachComponent<comp::Transform, comp::BoundingOrientedBox>([&, dt]
	(comp::Transform& transform, comp::BoundingOrientedBox& obb)
		{
			obb.Center = transform.position;
			/*obb.Orientation = transform.rotation;*/
		});

	//BoundingSphere
	scene.ForEachComponent<comp::Transform, comp::BoundingSphere>([&, dt]
	(comp::Transform& transform, comp::BoundingSphere& sphere)
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
	PROFILE_FUNCTION();

	scene.ForEachComponent<comp::NPC>([&](Entity entity, comp::NPC& npc)
	{
		comp::Transform* transformNPC = entity.GetComponent<comp::Transform>();
		Entity currentClosestPlayer;
		//npc.currentNode = FindClosestNode(scene, transformNPC->position);

		Entity closestPlayer = FindClosestPlayer(scene, transformNPC->position, &npc);
		comp::Velocity* velocityTowardsPlayer = entity.GetComponent<comp::Velocity>();
		comp::Transform* transformCurrentClosestPlayer = closestPlayer.GetComponent<comp::Transform>();
	/*	if (npc.currentNode)
		{
			if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= 100.f && npc.state != comp::NPC::State::CHASE)
			{
				npc.state = comp::NPC::State::CHASE;
				LOG_INFO("Switching to CHASE State!");
			}
			else if ((sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) >= npc.attackRange + 100.f && npc.state != comp::NPC::State::ASTAR))
			{
				npc.state = comp::NPC::State::ASTAR;
				LOG_INFO("Switching to ASTAR State!");
			}

			else if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= npc.attackRange)
			{
				comp::AttackAbility* stats = entity.GetComponent<comp::AttackAbility>();

				stats->targetDir = transformCurrentClosestPlayer->position - transformNPC->position;
				stats->targetDir.Normalize();
				stats->isAttacking = true;
			}
		}
		else
			npc.state = comp::NPC::State::IDLE;*/

		npc.state = comp::NPC::State::CHASE;

		if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= npc.attackRange)
		{
			comp::AttackAbility* stats = entity.GetComponent<comp::AttackAbility>();
			if (stats)
			{
				if (ecs::UseAbility(stats, transformCurrentClosestPlayer->position))
				{
					// Enemy Attacked
				};
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
			if (ReachedNode(&entity, npc.currentNode))
			{
				if (!npc.path.empty())
				{
					npc.currentNode = npc.path.at(0);
					npc.path.erase(npc.path.begin());
				}
				else
				{
					//npc.currentNode = FindClosestNode(scene, transformNPC->position);
					AIAStarSearch(entity, scene);
				}
			}
			else
			{
				if (velocityTowardsPlayer && npc.currentNode)
				{
					velocityTowardsPlayer->vel = npc.currentNode->position - transformNPC->position;
					velocityTowardsPlayer->vel.Normalize();
					velocityTowardsPlayer->vel *= npc.movementSpeed;
				}				
			}
			break;
		case comp::NPC::State::IDLE:
			velocityTowardsPlayer->vel = { 0.f, 0.f, 0.f };
			break;
		}
	});
}
