#pragma once
class Simulation;

/**Configurations for how the waves should behave.**/
struct WaveInfo
{
	int startNumOfEnemies;  ///< number of enemies wave system starts on.
	float spawnDistance;	///< The distance from the specified starting point the enemies should spawn from.
	int scaleMultiplier;	///< How the scaling for number of spawning enemies should develop after each wave
	int waveCount;			///< Keeps track of how many waves the system is up to
	float flankWidth;		///< Width of the flank
	sm::Vector2 origo;      ///< The point the wave is based on when spawning enemies
};

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
		Flank_West,  ///< Flank west spawns the enemy with a linear formation from the west, from the specified point
		Flank_East,  ///< Flank east spawns the enemy with a linear formation from the east, from the specified point
		Flank_North, ///< Flank north spawns the enemy with a linear formation from the north, from the specified point
		Flank_South, ///< Flank south spawns the enemy with a linear formation from the south, from the specified point
	};

	Entity CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyType type = EnemyType::Default);
	
}
/*! Namespace to manage the server's various ECS systems. */
namespace ServerSystems
{ 
	void WaveSystem(Simulation* simulation, std::queue<std::pair<EnemyManagement::WaveType, sm::Vector2>>& waves, WaveInfo& waveInfo);
	void RemoveDeadEnemies(Simulation* simulation);
}

namespace Systems {
	void CharacterMovement(HeadlessScene& scene, float dt);
};
