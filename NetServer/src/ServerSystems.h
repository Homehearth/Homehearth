#pragma once
class Wave;
class Simulation;

/*! Namespace with functions for managing the creation of enemies and types for identifying the different types of enemies and waves */
namespace EnemyManagement
{
	enum class EnemyType
	{
		Default,  //Basic walkere zombie
		Mage,     //Ranged enemy type
		Runner,   //Zombie but faster...
		BIGMOMMA, //Boss BIG MOMMA
		ENUM_SIZE
	};

	enum class WaveType : UINT8
	{
		Zone,	     ///< Zone will spawn the enemies around a specified point
		Swarm,	     ///< Swarm spawns the enemy so that they surround the specified point forming a circle
		ENUM_SIZE
	};

	struct EnemyGroup
	{
		std::vector<std::pair<EnemyManagement::EnemyType, int>> enemiesPerType;
		sm::Vector2 origo;      ///< The point the wave is based on when spawning enemies
	};
	
	Entity CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyType type = EnemyType::Default);

}

namespace PlayerManagement
{
	
}

/*! Namespace to manage the server's various ECS systems. */
namespace ServerSystems
{
	void WaveSystem(Simulation* simulation, std::queue<Wave>& waves);
	void NextWaveConditions(Simulation* simulation, Timer& timer, int timeToFinish);

	void UpdatePlayerWithInput(Simulation* simulation, HeadlessScene& scene, float dt);
	void PlayerStateSystem(Simulation* simulation, HeadlessScene& scene, float dt);
	
	void CheckGameOver(Simulation* simulation, HeadlessScene& scene);
	void TickBTSystem(Simulation* simulation, HeadlessScene& scene);

	void AnimatonSystem(Simulation* simulation, HeadlessScene& scene);
}
