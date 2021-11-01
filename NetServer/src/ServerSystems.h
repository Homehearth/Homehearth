#pragma once
class Simulation;

/*! Namespace with functions for managing the creation of enemies and types for identifying the different types of enemies and waves */
namespace EnemyManagement
{
	enum class EnemyType
	{
		Default
	};

	enum class WaveType : UINT8
	{
		Zone,	     ///< Zone will spawn the enemies around a specified point
		Swarm,	     ///< Swarm spawns the enemy so that they surround the specified point forming a circle
	};

	struct EnemyGroup
	{
		std::vector<std::pair<EnemyManagement::EnemyType, int>> enemiesPerType;
		sm::Vector2 origo;      ///< The point the wave is based on when spawning enemies
	};
	
	Entity CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyType type = EnemyType::Default);

}



/**Configurations for how the waves should behave.**/
struct WaveInfo
{
	std::vector<EnemyManagement::EnemyGroup> enemyGroups;
	float timerToFinish;
	float spawnDistance;	///< The distance from the specified starting point the enemies should spawn from.
	int flankWidth;			///< Width of the flank
};




/*! Namespace to manage the server's various ECS systems. */
namespace ServerSystems
{
	void WaveSystem(Simulation* simulation, std::queue<std::pair<EnemyManagement::WaveType, WaveInfo>>& waves);
	void ActivateNextWave(Simulation* simulation, Timer& timer, float timeToFinish);
}

namespace Systems {
	void CharacterMovement(HeadlessScene& scene, float dt);
};
