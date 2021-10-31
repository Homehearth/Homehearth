#include "NetServerPCH.h"
#include "ServerSystems.h"
#include "Simulation.h"

/**Creates an enemy entity.
 *
 *@param simulation Creates entity on current scene, and adds the network component
 *@param spawnP     Specifies the point at which the entity is to be created at
 *@param type       Specifies the type of enemy to create
 *@return Entity	returns the enemy that has been created as an Entity
 */
Entity EnemyManagement::CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyManagement::EnemyType type)
{
	Entity entity = simulation->GetGameScene()->CreateEntity();
	entity.AddComponent<comp::Network>()->id = simulation->m_pServer->PopNextUniqueID();
	entity.AddComponent<comp::Enemy>();
	entity.AddComponent<comp::Tag<DYNAMIC>>();
	
	comp::Transform*			transform   = entity.AddComponent<comp::Transform>();
	comp::Health*				health      = entity.AddComponent<comp::Health>();
	comp::MeshName*				meshName    = entity.AddComponent<comp::MeshName>();
	comp::BoundingOrientedBox*  obb         = entity.AddComponent<comp::BoundingOrientedBox>();
	comp::Velocity*				velocity    = entity.AddComponent<comp::Velocity>();
	comp::CombatStats*			combatStats = entity.AddComponent<comp::CombatStats>();

	switch (type)
	{
		case EnemyType::Default:
			{
			// ---DEFAULT ENEMY---
				transform->position = spawnP;
				meshName->name      = "Chest.obj";
				obb->Extents        = sm::Vector3(2.f, 2.f, 2.f);
				velocity->vel       = sm::Vector3(transform->position * -1.0f);
				velocity->vel.Normalize();
				velocity->vel		*= 20.0f;
				*combatStats        = { 1.0f, 20.f, 1.0f, false, false};
			}
	}

	return entity;
}



/**Wave system handles enemies spawns in the scene as a "wave", handles several different types of waves that is specified by the input queue.
 *@param simulation	   Manages sending and removal of entities on the server.
 *@param waves         Queue containing std::pair that describes which wave type to spawn and which position in the world the wave should be based on.
 *@param waveInfo      Contains configurations for the wave system.
 */
void ServerSystems::WaveSystem(Simulation* simulation, std::queue<std::pair<EnemyManagement::WaveType, sm::Vector2>>& waves, WaveInfo& waveInfo)
{
	//initialize a new wave
	if (!waves.empty())
	{
		const EnemyManagement::WaveType wave = waves.front().first;

		switch (wave)
		{
		//------------------------- ZONE ---------------------------------------
		case EnemyManagement::WaveType::Zone:
		{
			LOG_INFO("[WaveSystem] spawns a zone wave...");
			const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
			const int min = 30;
			const int max = 70;

			const float deg2rad = 3.14f / 180.f;
			const float degree = (rand() % 360) * deg2rad;

			// for each enemy i:
			for (int i = 0; i < nrOfEnemies; i++)
			{
				const float distance = (rand() % (max - min)) + min;

				const float posX = distance * cos(i * degree);
				const float posZ = distance * sin(i * degree);

				simulation->SendEntity(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default));
			}
		}
		break;
		//------------------------- END ---------------------------------------
		
		//------------------------- Swarm ---------------------------------------
		case EnemyManagement::WaveType::Swarm:
		{
			LOG_INFO("[WaveSystem] spawns a swarm wave...");
			const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
			const float distance = waveInfo.spawnDistance;

			const float deg2rad = 3.14f / 180.f;
			const float degree = (360.f / static_cast<float>(nrOfEnemies)) * deg2rad;

			// for each enemy i:
			for (int i = 0; i < nrOfEnemies; i++)
			{
				const float posX = distance * cos(i * degree);
				const float posZ = distance * sin(i * degree);

				simulation->SendEntity(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default));
			}
		}
		break;
		//------------------------- END ---------------------------------------

		//------------------------- Flank West ---------------------------------------
		case EnemyManagement::WaveType::Flank_West:
		{
			LOG_INFO("[WaveSystem] spawns a flank west wave...");
			const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
			const float distance = waveInfo.spawnDistance;
			const int enemyOffset = waveInfo.flankWidth / nrOfEnemies;
			for (int i = 0; i < nrOfEnemies; i++)
			{

				float posX = -distance;
				float posZ = (-1.f * waveInfo.flankWidth / 2) + float(i * enemyOffset);

				simulation->SendEntity(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default));
			}
		}
		break;
		//------------------------- END ---------------------------------------

		//------------------------- Flank East ---------------------------------------
		case EnemyManagement::WaveType::Flank_East:
		{
			LOG_INFO("[WaveSystem] spawns a flank east wave...");
			const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
			const float distance = waveInfo.spawnDistance;
			const int enemyOffset = waveInfo.flankWidth / nrOfEnemies;
			for (int i = 0; i < nrOfEnemies; i++)
			{

				float posX = distance;
				float posZ = (-1.f * waveInfo.flankWidth / 2) + float(i * enemyOffset);

				simulation->SendEntity(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default));
			}
		}
		break;
		//------------------------- END ---------------------------------------

		//------------------------- Flank North ---------------------------------------
		case EnemyManagement::WaveType::Flank_North:
		{
			LOG_INFO("[WaveSystem] spawns a flank north wave...");
			bool bUniform = false;
			const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
			const float distance = waveInfo.spawnDistance;

			const int enemyOffset = waveInfo.flankWidth / nrOfEnemies;

			for (int i = 0; i < nrOfEnemies; i++)
			{

				float posX = (-1.f * waveInfo.flankWidth / 2) + float(i * enemyOffset);
				float posZ = distance;

				simulation->SendEntity(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default));
			}
		}
		break;
		//------------------------- END ---------------------------------------

		//------------------------- Flank South ---------------------------------------
		case EnemyManagement::WaveType::Flank_South:
		{
			LOG_INFO("[WaveSystem] spawns a flank south wave...");
			const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
			const float distance = waveInfo.spawnDistance;
			const int enemyOffset = waveInfo.flankWidth / nrOfEnemies;
			for (int i = 0; i < nrOfEnemies; i++)
			{

				float posX = (-1.f * waveInfo.flankWidth / 2) + float(i * enemyOffset);
				float posZ = -distance;

				simulation->SendEntity(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default));
			}
		}
		break;
		//------------------------- END ---------------------------------------

		//------------------------- Default ---------------------------------------
		default:
		{
			LOG_WARNING("[ServerSystems]::WaveSystem failed to initialize a wave spawn, [Controll the wave type]");
		}
		break;
		//------------------------- END ---------------------------------------
			
		}

		//Add count and pop from queue
		waveInfo.waveCount++;
		waves.pop();
	}
}



/**Removes all enemies that has been destroyed and broadcasts the removal to the clients.
 *@param simulation	   Manages sending and removal of entities on the server.
 */
void ServerSystems::RemoveDeadEnemies(Simulation* simulation)
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;
	
	int count = 0;
	int numOfEnemies = 0;
	
	//Summarize all the existing enemy components in the scene
	simulation->GetGameScene()->ForEachComponent<comp::Enemy, comp::Network, comp::Transform>([&](Entity entity, comp::Enemy enemy, comp::Network network, comp::Transform transform)
		{
			if (abs(transform.position.x) < 8.f && abs(transform.position.z) < 8.f || abs(transform.position.x) > 100.f || abs(transform.position.z) > 100.f)
			{
				msg << network.id;
				count++;
				entity.Destroy();
			}
			else
			{
				numOfEnemies++;
			}
		});

	//Send out which entities have been destroyed to the clients
	if (count > 0)
	{
		msg << count;
		simulation->SendRemoveEntities(msg);
	}

	//Publish all enemies have been destroyed
	if(numOfEnemies == 0)
	{
		simulation->GetGameScene()->publish<ESceneEnemiesWiped>(0.0f);
	}
}

namespace Systems {
	void CharacterMovement(HeadlessScene& scene, float dt)
	{
		scene.ForEachComponent<comp::Player, comp::CombatStats, comp::Velocity, comp::Transform>([&](comp::Player& p, comp::CombatStats& a, comp::Velocity& v, comp::Transform& t)
			{
				if (a.isAttacking) // should only happen one frame
				{
					
					Plane_t plane;
					plane.normal = sm::Vector3(0, 1, 0);
					plane.point = t.position;

					sm::Vector3 point(0, 0, 0);
					sm::Vector3 targetDir(1, 0, 0);

					if (a.targetRay.Intersects(plane, point))
					{
						targetDir = point - t.position;
						targetDir.Normalize(targetDir);
					}
					else {
						LOG_WARNING("Mouse click ray missed walking plane. Should not happen...");
					}

					p.state = comp::Player::State::ATTACK;
					p.targetForward = targetDir;
				}

				if (p.state == comp::Player::State::ATTACK) // happens every frame the player is attacking
					v.vel = sm::Vector3::Zero;

			});
		// turns player with velocity
		scene.ForEachComponent<comp::Player, comp::Velocity>([&](comp::Player& p, comp::Velocity& v)
			{
				if (v.vel.Length() > 0.001f && p.state != comp::Player::State::ATTACK)
				{
					sm::Vector3 vel;
					v.vel.Normalize(vel);
					
					p.state = comp::Player::State::TURN;
					p.targetForward = vel;
				}				
			});

		// if player is turning, turns until forward is reached
		scene.ForEachComponent<comp::Player, comp::Transform>([&](comp::Player& p, comp::Transform& t)
			{
				if (p.state == comp::Player::State::TURN || p.state == comp::Player::State::ATTACK)
				{
					float time = dt * p.runSpeed;
					if (ecs::StepRotateTo(t.rotation, p.targetForward, time))
					{
						p.state = comp::Player::State::IDLE;
					}
				}
			});
	}
}
