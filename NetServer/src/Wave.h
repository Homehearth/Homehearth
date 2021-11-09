#pragma once
#include "ServerSystems.h"

// This class holds specific information about a wave.
// The WaveSystem will be using this information.
class Wave
{
public:
	Wave();
	virtual ~Wave() = default;

	// Waves are divided into groups.
	// This class holds specific information about a group.
	class Group
	{
	private:
		std::unordered_map<EnemyManagement::EnemyType, int> m_enemies;
		std::unordered_map<EnemyManagement::EnemyType, int>::iterator m_iterator;
		sm::Vector2 m_spawnPoint;
		int m_totalEnemyCount = 0;

	public:
		Group() = default;
		virtual ~Group() = default;

		// Add 'count' number of enemies of 'type'.
		void AddEnemy(EnemyManagement::EnemyType type, int count);

		// Set the spawn point for the group.
		void SetSpawnPoint(const sm::Vector2& origin);

		// Retrieve spawn point for the group.
		sm::Vector2 GetSpawnPoint() const;

		// Retrieve the number of enemies of given 'type'.
		int GetEnemyTypeCount(const EnemyManagement::EnemyType& type);

		// Get the total number of enemies in the group.
		int GetTotalNumberOfEnemies() const;


		void ForEachEnemyType(std::function<void()>& func);

	}; // end of class Group.

	// Get a vector containing all groups belonging to the wave.
	std::vector<Group>& GetGroups();

	// Add a new group to the wave.
	void AddGroup(const Group& group);

	// Set the wave spawn distance from origin.
	void SetDistance(const float& distance);

	// Set the width of the wave upon spawn.
	void SetFlankWidth(const float& width);

	// Set how long the wave will last in seconds. 
	void SetTimeLimit(int time);

	// Get the time limit of the wave.
	int GetTimeLimit() const;

	// Retrieve the 'WaveType' of the wave.
	EnemyManagement::WaveType GetWaveType() const;

	// Get the wave spawn distance from origin.
	float GetDistance() const;

	// Get the width of the wave upon spawn.
	float GetFlankWidth() const;


private:
	std::vector<Group> m_groups;
	EnemyManagement::WaveType m_waveType;
	int m_timeLimit;

	float m_distance;
	float m_flankWidth;

}; // end of class Wave.


