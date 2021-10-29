#include "NetServerPCH.h"
#include "ServerSystems.h"
//Creates an entity of enemy type 1
Entity EnemyManagement::CreateEnemy(HeadlessScene& scene, Server* server, sm::Vector3 spawnP, EnemyManagement::EnemyType type)
{
	Entity entity = scene.CreateEntity();
	entity.AddComponent<comp::Network>()->id = server->PopNextUniqueID();
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
				velocity->vel		*= 3.0f;
				*combatStats        = { 1.0f, 20.f, 1.0f, false, false};
			}
	}

	return entity;
}

void EnemyManagement::SpawnSwarmWave(HeadlessScene& scene, Server* server, WaveInfo& waveInfo)
{
	LOG_INFO("[WaveSystem] spawns a swarm wave...");
	const int nrOfEnemies = waveInfo.startNumOfEnemies + (waveInfo.scaleMultiplier * waveInfo.waveCount);
	const float radius	  = 50.f;
	
	const float deg2rad = 3.14f / 180.f;
	const float degree	= (360.f / (float)nrOfEnemies) * deg2rad;
	
	// for each enemy i:
	for(int i = 0; i < nrOfEnemies; i++)
	{
		float posX = radius * cos(i * degree);
		float posZ = radius * sin(i * degree);
		
		EnemyManagement::CreateEnemy(scene, server, { posX, 0.0f, posZ }, EnemyType::Default);
	}

}

void ServerSystems::WaveSystem(HeadlessScene& scene, Server* server, std::vector<EnemyManagement::WaveType> waves)
{
	int numOfEnemies = 0;
	
	static WaveInfo waveInfo =
	{
		waveInfo.startNumOfEnemies = 20,
		waveInfo.spawnDistance = 5.f,
		waveInfo.scaleMultiplier = 2,
		waveInfo.waveCount = 0,
	};

	scene.ForEachComponent<comp::Enemy, comp::Transform>([&](Entity entity, comp::Enemy enemy, comp::Transform transform)
		{
			if(abs(transform.position.x) < 3.f && abs(transform.position.z) < 3.f)
			{
				entity.Destroy();
			}
			else
				numOfEnemies++;
		});

	//initialize a new wave
	if (numOfEnemies == 0)
	{
		const EnemyManagement::WaveType wave = waves.back();
		switch (wave)
		{
		case EnemyManagement::WaveType::Swarm:
			{
				EnemyManagement::SpawnSwarmWave(scene, server, waveInfo);
			}
			break;
			
		case EnemyManagement::WaveType::Flank_West:
			{
				
			}
			break;
			
		case EnemyManagement::WaveType::Flank_East:
			{
				
			}
			break;
			
		case EnemyManagement::WaveType::Flank_North:
			{
				
			}
			break;
			
		case EnemyManagement::WaveType::Flank_South:
			{
				
			}
			break;
			
		default:
			{
				
			}
			break;
		}
		
		waveInfo.waveCount++;
		waves.pop_back();
	}
}
