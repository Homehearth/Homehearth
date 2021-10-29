#include "NetServerPCH.h"
#include "ServerSystems.h"
#include "Simulation.h"
//Creates an entity of enemy type 1
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
				velocity->vel		*= 10.0f;
				*combatStats        = { 1.0f, 20.f, 1.0f, false, false};
			}
	}

	return entity;
}

void ServerSystems::WaveSystem(Simulation* simulation, std::queue<std::pair<EnemyManagement::WaveType, sm::Vector2>>& waves)
{
	int numOfEnemies = 0;
	
	static WaveInfo waveInfo =
	{
		waveInfo.startNumOfEnemies = 3,
		waveInfo.spawnDistance = 100.f,
		waveInfo.scaleMultiplier = 2,
		waveInfo.waveCount = 0,
		waveInfo.flankWidth = 100.f
	};
	
	simulation->GetGameScene()->ForEachComponent<comp::Enemy, comp::Transform>([&](Entity entity, comp::Enemy enemy, comp::Transform transform)
		{
			if(abs(transform.position.x) < 8.f && abs(transform.position.z) < 8.f || abs(transform.position.x) > 500.f || abs(transform.position.z) > 500.f)
			{
				entity.Destroy();
			}
			else
				numOfEnemies++;
		});

	//initialize a new wave
	if (numOfEnemies == 0)
	{
		const EnemyManagement::WaveType wave = waves.front().first;
		switch (wave)
		{
		case EnemyManagement::WaveType::Zone:
			{
				LOG_INFO("[WaveSystem] spawns a zone wave...");
				const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
				int min = 30;
				int max = 70;
				
				const float deg2rad = 3.14f / 180.f;
				const float degree = (rand() % 360) * deg2rad;

				// for each enemy i:
				for (int i = 0; i < nrOfEnemies; i++)
				{
					const float distance = (rand() % (max - min)) + min;
					
					float posX = distance * cos(i * degree);
					float posZ = distance * sin(i * degree);

					simulation->Broadcast(simulation->SingleEntityMessage(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default)));
				}
			}
		break;
		case EnemyManagement::WaveType::Swarm:
			{
				LOG_INFO("[WaveSystem] spawns a swarm wave...");
				const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
				const float distance = waveInfo.spawnDistance;

				const float deg2rad = 3.14f / 180.f;
				const float degree = (360.f / (float)nrOfEnemies) * deg2rad;

				// for each enemy i:
				for (int i = 0; i < nrOfEnemies; i++)
				{
					float posX = distance * cos(i * degree);
					float posZ = distance * sin(i * degree);

					simulation->Broadcast(simulation->SingleEntityMessage(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default)));
				}
			}
			break;
			
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

					simulation->Broadcast(simulation->SingleEntityMessage(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default)));
				}
			}
			break;
			
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

					simulation->Broadcast(simulation->SingleEntityMessage(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default)));
				}
			}
			break;
			
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

					simulation->Broadcast(simulation->SingleEntityMessage(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default)));
				}
			}
			break;
			
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

					simulation->Broadcast(simulation->SingleEntityMessage(EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, EnemyManagement::EnemyType::Default)));
				}
			}
			break;
			
		default:
			{
				
			}
			break;
		}

		
	
		waveInfo.waveCount++;
		waves.pop();
	}
}
