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
	sm::Vector2 origo;
};

namespace EnemyManagement
{
	enum class EnemyType
	{
		Default
	};

	enum class WaveType : UINT8
	{
		Zone,
		Swarm,
		Flank_West,
		Flank_East,
		Flank_North,
		Flank_South,
	};

	Entity CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyType type = EnemyType::Default);
	
}

namespace ServerSystems
{
	void WaveSystem(Simulation* simulation, std::queue<std::pair<EnemyManagement::WaveType, sm::Vector2>>& waves);
}

namespace Systems {
	void CharacterMovement(HeadlessScene& scene, float dt);

};
