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





void Systems::CombatSystem(HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();

	scene.ForEachComponent<comp::CombatStats>([&](comp::CombatStats& stats)
		{
			// Decreases cooldown between attacks.
			if (stats.delayTimer > 0.f)
				stats.delayTimer -= dt;

			if (stats.cooldownTimer > 0.f)
				stats.cooldownTimer -= dt;
			else
				stats.isReady = true;
		});

	// For Each entity that can attack.
	scene.ForEachComponent<comp::CombatStats, comp::Transform>([&](Entity entity, comp::CombatStats& stats, comp::Transform& transform)
		{
			//
			// attack LOGIC
			//
			if (stats.isUsing && stats.delayTimer <= 0.f)
			{

				//Creates an entity that's used to check collision if an attack lands.
				Entity attackCollider = scene.CreateEntity();
				comp::Transform* t = attackCollider.AddComponent<comp::Transform>();
				comp::BoundingOrientedBox* box = attackCollider.AddComponent<comp::BoundingOrientedBox>();

				box->Extents = sm::Vector3(stats.attackRange * 0.5f);

				stats.targetDir.Normalize();
				t->position = transform.position + stats.targetDir * stats.attackRange * 0.5f;
				t->rotation = transform.rotation;

				box->Center = t->position;
				box->Orientation = t->rotation;


				comp::SelfDestruct* selfDestruct = attackCollider.AddComponent<comp::SelfDestruct>();
				selfDestruct->lifeTime = stats.lifetime;

				//If the attack is ranged add a velocity to the entity.
				if (stats.isRanged)
				{
					sm::Vector3 vel = stats.targetDir * stats.projectileSpeed;
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
						comp::CombatStats* stats = entity.GetComponent<comp::CombatStats>();

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

				stats.cooldownTimer = stats.cooldown;
				stats.isUsing = false;
			}

		});


	//Health System
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
				}

			}
		});

	//Projectile Life System
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

	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::TemporaryPhysics >([&](Entity e, comp::Transform& t, comp::Velocity& v, comp::TemporaryPhysics& p)
		{
			v.vel = v.oldVel; // ignore any changes made to velocity made this frame
			auto& it = p.forces.begin();
			while (it != p.forces.end())
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

void Systems::AISystem(HeadlessScene& scene, AIHandler* aiHandler)
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

			Node* closestNod = aiHandler->FindClosestNode(transformCurrentClosestPlayer->position);

			if (sm::Vector3::Distance(transformNPC->position, transformCurrentClosestPlayer->position) <= npc.attackRange)
			{
				comp::CombatStats* stats = entity.GetComponent<comp::CombatStats>();

				stats->targetDir = transformCurrentClosestPlayer->position - transformNPC->position;
				stats->targetDir.Normalize();
				stats->targetDir *= 10.f;
				if (ecs::Use(stats))
				{
					// Enemy Attacked
				};
			}


			switch (npc.state)
			{
			case comp::NPC::State::ASTAR:

				if(npc.path.size() > 0)
				{
					npc.currentNode = npc.path.back();
					if (velocityTowardsPlayer && npc.currentNode)
					{
						velocityTowardsPlayer->vel = npc.currentNode->position - transformNPC->position;
						velocityTowardsPlayer->vel.Normalize();
						velocityTowardsPlayer->vel *= npc.movementSpeed;
					}

					if(sm::Vector3::Distance(npc.currentNode->position, transformNPC->position) < 8.f)
					{
						npc.path.pop_back();
					}
					
				}
				else
				{
					npc.currentNode = aiHandler->FindClosestNode(transformNPC->position);
					aiHandler->AStarSearch(entity);
					velocityTowardsPlayer->vel = sm::Vector3(0.f,0.f,0.f);
				}


			}
		});
}

void Systems::UpdatePlayerVisuals(Scene* scene)
{
	int i = 1;
	scene->ForEachComponent<comp::NamePlate, comp::Transform>([&](Entity& e, comp::NamePlate& name, comp::Transform& t)
		{
			if (i < 5)
			{
				Collection2D* collection = scene->GetCollection("dynamicPlayer" + std::to_string(i) + "namePlate");
				if (collection)
				{
					rtd::Text* namePlate = dynamic_cast<rtd::Text*>(collection->elements[0].get());
					if (namePlate)
					{
						Camera* cam = scene->GetCurrentCamera();

						if (cam->GetCameraMatrixes())
						{
							// Conversion from World space to NDC space.
							sm::Vector4 oldP = { t.position.x, t.position.y + 25.0f, t.position.z, 1.0f };
							sm::Vector4 newP = dx::XMVector4Transform(oldP, cam->GetCameraMatrixes()->view);
							newP = dx::XMVector4Transform(newP, cam->GetCameraMatrixes()->projection);
							newP.x /= newP.w;
							newP.y /= newP.w;
							newP.z /= newP.w;

							// Conversion from NDC space [-1, 1] to Window space
							float new_x = (((newP.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
							float new_y = D2D1Core::GetWindow()->GetHeight() - (((newP.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

							namePlate->SetPosition(new_x - ((name.namePlate.length() * D2D1Core::GetDefaultFontSize()) * 0.5f), new_y);
							// Show nameplates only if camera is turned to it.
							if (newP.z < 1.f)
								namePlate->SetVisiblity(true);
							else
								namePlate->SetVisiblity(false);

							// Update healthbars position.
							Collection2D* collHealth = scene->GetCollection("player" + std::to_string(i) + "Info");
							if (collHealth)
							{
								rtd::Healthbar* health = dynamic_cast<rtd::Healthbar*>(collHealth->elements[0].get());
								if (health)
								{
									sm::Vector4 oldPp = { t.position.x, t.position.y + 20.0f, t.position.z, 1.0f };
									sm::Vector4 newPp = dx::XMVector4Transform(oldPp, cam->GetCameraMatrixes()->view);
									newPp = dx::XMVector4Transform(newPp, cam->GetCameraMatrixes()->projection);
									newPp.x /= newPp.w;
									newPp.y /= newPp.w;
									newPp.z /= newPp.w;

									// Conversion from NDC space [-1, 1] to Window space
									new_x = (((newPp.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
									new_y = D2D1Core::GetWindow()->GetHeight() - (((newPp.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

									health->SetPosition(new_x - (health->GetOpts().width * 0.5f), new_y);

									// Only visible if camera is turned to it.
									if (newPp.z < 1.f)
										health->SetVisiblity(true);
									else
										health->SetVisiblity(false);
								}
							}
						}
					}
				}
			}
			i++;
		});

}
