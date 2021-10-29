#pragma once
#include "Server.h"
//do maybe move this namespace?
struct WaveInfo
{
	int startNumOfEnemies;
	float spawnDistance;
	int scaleMultiplier;
	int waveCount;
};

namespace EnemyManagement
{
	enum class EnemyType
	{
		Default
	};

	enum class WaveType : UINT8
	{
		Swarm,
		Flank_West,
		Flank_East,
		Flank_North,
		Flank_South,
	};

	Entity CreateEnemy(HeadlessScene& scene, Server* server, sm::Vector3 spawnP, EnemyType type = EnemyType::Default);

	void SpawnSwarmWave(HeadlessScene& scene, Server* server, WaveInfo& waveInfo);
	
}

namespace ServerSystems
{
	void WaveSystem(HeadlessScene& scene, Server* server, std::vector<EnemyManagement::WaveType> waves);
}