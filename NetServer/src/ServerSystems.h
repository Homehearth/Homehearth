#pragma once
class Simulation;
//do maybe move this namespace?
struct WaveInfo
{
	int startNumOfEnemies;
	float spawnDistance;
	int scaleMultiplier;
	int waveCount;
	float flankWidth;
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

	Entity CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyType type = EnemyType::Default);

	void SpawnSwarmWave(Simulation* simulation, WaveInfo& waveInfo);
	
}

namespace ServerSystems
{
	void WaveSystem(Simulation* simulation, std::vector<EnemyManagement::WaveType>& waves);
}