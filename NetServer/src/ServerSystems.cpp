#include "NetServerPCH.h"
#include "Wave.h"
#include "ServerSystems.h"
#include "Simulation.h"


/**Creates an enemy entity at specified point.
 *
 *@param simulation Creates entity on current scene, and adds the network component
 *@param spawnP     Specifies the point at which the entity is to be created at
 *@param type       Specifies the type of enemy to create
 *@return Entity	returns the enemy that has been created as an Entity
 */
Entity EnemyManagement::CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyType type)
{
	Entity entity = simulation->GetGameScene()->CreateEntity();
	entity.AddComponent<comp::Network>();
	entity.AddComponent<comp::NPC>();
	entity.AddComponent<comp::Tag<DYNAMIC>>();

	comp::Transform* transform = entity.AddComponent<comp::Transform>();
	comp::Health* health = entity.AddComponent<comp::Health>();
	comp::MeshName* meshName = entity.AddComponent<comp::MeshName>();
	comp::BoundingOrientedBox* obb = entity.AddComponent<comp::BoundingOrientedBox>();
	//comp::BoundingSphere* bos = entity.AddComponent<comp::BoundingSphere>();
	comp::Velocity* velocity = entity.AddComponent<comp::Velocity>();
	comp::AttackAbility* combatStats = entity.AddComponent<comp::AttackAbility>();
	comp::BehaviorTree* behaviorTree = entity.AddComponent<comp::BehaviorTree>();
	switch (type)
	{
	case EnemyType::Default:
		{
			// ---DEFAULT ENEMY---
			transform->position = spawnP;
			transform->scale = { 1.8f, 1.8f, 1.8f };
			meshName->name = "Monster.fbx";
			entity.AddComponent<comp::AnimatorName>()->name = "Monster.anim";
			obb->Extents = sm::Vector3(2.f, 2.f, 2.f);
			//bos->Radius = 2.f;
			/*velocity->vel = sm::Vector3(transform->position * -1.0f);
			velocity->vel.Normalize();
			velocity->vel *= 5.0f;*/
			combatStats->cooldown = 1.0f;
			combatStats->attackDamage = 20.f;
			combatStats->lifetime = 0.2f;
			combatStats->isRanged = false;
			behaviorTree->root = AIBehaviors::GetSimpleAIBehavior(entity);
			combatStats->attackRange = 7.0f;

			
		}
		break;
	case EnemyType::Default2:
		{
			// ---DEFAULT ENEMY 2---
			transform->position = spawnP;
			meshName->name = "Barrel.obj";
			transform->scale = { 1.8f, 1.8f, 1.8f };
			obb->Extents = sm::Vector3(2.f, 2.f, 2.f);
			//bos->Radius = 2.f;
			/*velocity->vel = sm::Vector3(transform->position * -1.0f);
			velocity->vel.Normalize();
			velocity->vel *= 5.0f;*/
			combatStats->cooldown = 1.0f;
			combatStats->attackDamage = 20.f;
			combatStats->lifetime = 0.2f;
			combatStats->isRanged = false;
			combatStats->attackRange = 7.0f;

		}
		break;
	default:
			LOG_WARNING("Attempted to create unknown EnemyType.")
		break;
	}

	return entity;
}



/**Spawn the enemies randomly within a circular zone based on the specified point
 *
 *@param simulation	   Manages sending and removal of entities on the server.
 *@param point		   This is the point the calculation assumes to spawn the enemy from.
 *@param waveInfo      Contains configurations for the wave system.
 */
void SpawnZoneWave(Simulation* simulation, Wave& currentWave)
{
	LOG_INFO("[WaveSystem] spawns a zone wave...");
	
	const int min = 30;
	const int max = 70;
	const float deg2rad = 3.14f / 180.f;
	int nrOfEnemies = 0;
	float degree = 0.0f;

	float distance;
	float posX;
	float posZ;

	using namespace EnemyManagement;
	for (auto& group : currentWave.GetGroups())
	{
		//Go through each enemyType of that group
		constexpr int SIZE = static_cast<int>(EnemyType::ENUM_SIZE);
		for (auto it = 0; it < SIZE; it++)
		{
			nrOfEnemies = group.GetEnemyTypeCount(static_cast<EnemyType>(it));
			const float degree = (360.f / static_cast<float>(nrOfEnemies)) * deg2rad;

			//Spawn enemies of this type
			for (int i = 0; i < nrOfEnemies; i++)
			{
				distance = static_cast<float>(rand() % (max - min) + min);
				posX = distance * cos(i * degree) + group.GetSpawnPoint().x;
				posZ = distance * sin(i * degree) + group.GetSpawnPoint().y;
				EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, static_cast<EnemyType>(it));
			}
		}
	}
}

/**Spawn the enemies within a circular zone based on the specified point
 *
 *@param simulation	   Manages sending and removal of entities on the server.
 *@param point		   This is the point the calculation assumes to spawn the enemy from.
 *@param waveInfo      Contains configurations for the wave system.
 */
void SpawnSwarmWave(Simulation* simulation, Wave& currentWave)
{
	LOG_INFO("[WaveSystem] spawns a swarm wave...");
	
	const float distance = currentWave.GetDistance();
	const float deg2rad = 3.14f / 180.f;
	int nrOfEnemies = 0;

	float posX;
	float posZ;

	using namespace EnemyManagement;
	for (auto& group : currentWave.GetGroups())
	{
		//Go through each enemyType of that group
		constexpr int SIZE = static_cast<int>(EnemyType::ENUM_SIZE);
		for (auto it = 0; it < SIZE; it++)
		{
			nrOfEnemies = group.GetEnemyTypeCount(static_cast<EnemyType>(it));
			const float degree = (360.f / static_cast<float>(nrOfEnemies)) * deg2rad;

			for (int i = 0; i < nrOfEnemies; i++)
			{
				posX = distance * cos(i * degree) + group.GetSpawnPoint().x;
				posZ = distance * sin(i * degree) + group.GetSpawnPoint().y;
				EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, static_cast<EnemyType>(it));
			}
		}
	}
}

/**Wave system handles enemies spawns in the scene as a "wave", handles several different types of waves that is specified by the input queue.
 *
 *@param simulation	   Manages sending and removal of entities on the server.
 *@param waves         Queue containing std::pair that describes which wave type to spawn and which position in the world the wave should be based on.
 *@param waveInfo      Contains configurations for the wave system.
 */
void ServerSystems::WaveSystem(Simulation* simulation,
                               std::queue<Wave>& waves)
{
	//initialize a new wave
	if (!waves.empty())
	{
		const EnemyManagement::WaveType waveType = waves.front().GetWaveType();

		switch (waveType)
		{
		case EnemyManagement::WaveType::Zone:
			{
				SpawnZoneWave(simulation, waves.front());
			}
			break;

		case EnemyManagement::WaveType::Swarm:
			{
				SpawnSwarmWave(simulation, waves.front());
			}
			break;

		default:
			{
				LOG_WARNING("[ServerSystems]::WaveSystem failed to initialize a wave spawn, [Controll the wave type]");
			}
			break;
		}

		//Add count and pop from queue
		waves.pop();
	}
}


/**Removes all enemies that has been destroyed and broadcasts the removal to the clients.
 *@param simulation	   Manages sending and removal of entities on the server.
 */
void ServerSystems::NextWaveConditions(Simulation* simulation, Timer& timer, int timeToFinish)
{
	//Publish event when timeToFinish been exceeded.
	if (timer.GetElapsedTime() > timeToFinish)
	{
		simulation->GetGameScene()->publish<ESceneCallWaveSystem>(0.0f);
	}
}

void ServerSystems::UpdatePlayerWithInput(Simulation* simulation, HeadlessScene& scene, float dt)
{
	scene.ForEachComponent<comp::Player, comp::Transform, comp::Velocity>([](comp::Player& p, comp::Transform& t, comp::Velocity& v)
		{
			// update velocity
			sm::Vector3 vel = sm::Vector3(static_cast<float>(p.lastInputState.axisHorizontal), 0, static_cast<float>(p.lastInputState.axisVertical));
			vel.Normalize();

			sm::Vector3 cameraToPlayer = t.position - p.lastInputState.mouseRay.origin;
			cameraToPlayer.y = 0;
			cameraToPlayer.Normalize();
			float targetRotation = atan2(-cameraToPlayer.x, -cameraToPlayer.z);
			vel = sm::Vector3::TransformNormal(vel, sm::Matrix::CreateRotationY(targetRotation));

			if (vel.Length() > 0.01f)
				p.state = comp::Player::State::WALK;

			vel *= p.runSpeed;
			v.vel = vel;
		});

	scene.ForEachComponent<comp::Player>([&](Entity e, comp::Player& p)
		{
			// Do stuff based on input

			// Get point on ground where mouse hovers over
			Plane_t plane;
			plane.normal = sm::Vector3(0, 1, 0);
			plane.point = sm::Vector3(0, 0, 0);

			if (!p.lastInputState.mouseRay.Intersects(plane, &p.mousePoint))
			{
				LOG_WARNING("Mouse click ray missed walking plane. Should not happen...");
			}

			// check if using abilities
			if (p.lastInputState.leftMouse) // is held
			{
				p.state = comp::Player::State::LOOK_TO_MOUSE; // set state even if ability is not ready for use yet
				if (ecs::UseAbility(e, p.primaryAbilty, &p.mousePoint))
				{

				}
			}
			else if (p.lastInputState.rightMouse) // was pressed
			{
				LOG_INFO("Pressed right");
				if (ecs::UseAbility(e, p.secondaryAbilty, &p.mousePoint))
				{

				}
			}

			//Place defence on grid
			if (p.lastInputState.key_b && simulation->GetCurrency().GetAmount() >= 20)
				if (simulation->GetGrid().PlaceDefence(p.lastInputState.mouseRay, e.GetComponent<comp::Network>()->id, Blackboard::Get().GetAIHandler()))
					simulation->GetCurrency().GetAmountRef() -= 20;

		});


}

void ServerSystems::PlayerStateSystem(Simulation* simulation, HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();
	scene.ForEachComponent<comp::Player>([](Entity e, comp::Player& p)
		{
			if (ecs::IsPlayerUsingAnyAbility(e))
			{
				p.state = comp::Player::State::LOOK_TO_MOUSE; // always use this state if any ability is being used
			}

		});

	scene.ForEachComponent<comp::Player, comp::Health>([&](Entity e, comp::Player& p, comp::Health& health)
		{
			if(!health.isAlive)
			{
				comp::Velocity* vel = e.GetComponent<comp::Velocity>();
				if (vel)
				{
					// dont move if dead
					vel->vel = sm::Vector3::Zero;
				}

				p.respawnTimer -= dt;

				if(p.respawnTimer < 0.01f)
				{
					simulation->ResetPlayer(e);

					LOG_INFO("Player id %u Respawnd...", e.GetComponent<comp::Network>()->id);
				}
			}
		});

	scene.ForEachComponent<comp::Player, comp::Velocity, comp::Transform>([&](comp::Player& p, comp::Velocity& v, comp::Transform& t)
		{
			if (p.state == comp::Player::State::LOOK_TO_MOUSE)
			{
				// turns player to look at mouse world pos
				p.fowardDir = p.mousePoint - t.position;
				p.fowardDir.Normalize();
				v.vel *= 0.2f;
			}
			else if (p.state == comp::Player::State::WALK)
			{
				// turns player with velocity
				sm::Vector3 dir;
				v.vel.Normalize(dir);
				p.fowardDir = dir;
			}
		});
	
	// if player is turning, turns until forward is reached
	scene.ForEachComponent<comp::Player, comp::Transform>([&](Entity e, comp::Player& p, comp::Transform& t)
		{
			if (p.state == comp::Player::State::WALK || p.state == comp::Player::State::LOOK_TO_MOUSE)
			{
				float time = dt * p.runSpeed * 0.5f;
				
				if (ecs::StepRotateTo(t.rotation, p.fowardDir, time))
				{
					p.state = comp::Player::State::IDLE;
				}
				e.UpdateNetwork();
			}
		});

	// turns npc with velocity
	scene.ForEachComponent<comp::NPC, comp::Transform, comp::Velocity>([&](Entity e, comp::NPC& p, comp::Transform& t, comp::Velocity& v)
		{
			if (v.vel.Length() > 0.001f)
			{
				float time = dt * p.movementSpeed * 0.5f;
				if (ecs::StepRotateTo(t.rotation, v.vel, time))
				{

				}
				e.UpdateNetwork();
			}
		});
}

void ServerSystems::CheckGameOver(Simulation* simulation, HeadlessScene& scene)
{
	PROFILE_FUNCTION();

	bool gameOver = true;
	
	//Check if all players is dead
	scene.ForEachComponent<comp::Player, comp::Health>([&](comp::Player& p, comp::Health& h)
		{
			if (h.isAlive)
			{
				gameOver = false;
			}
		});

	if (gameOver)
	{
		simulation->SetLobbyScene();
	}
}

void ServerSystems::TickBTSystem(Simulation* simulation, HeadlessScene& scene)
{
	scene.ForEachComponent<comp::BehaviorTree>([&](Entity entity,comp::BehaviorTree& bt)
		{
			if(bt.currentNode != nullptr)
			{
				bt.currentNode->Tick();
			}
			else
			{
				bt.root->Tick();
			}
		
		});
}
