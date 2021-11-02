#include "NetServerPCH.h"
#include "Wave.h"

// class Group.
void Wave::Group::AddEnemy(const EnemyManagement::EnemyType type, int count)
{
	m_iterator = m_enemies.find(type);
	if (m_iterator != m_enemies.end())
		m_iterator->second += count;
	else
		m_enemies.insert({type, count});

	m_totalEnemyCount += count;
}

void Wave::Group::SetSpawnPoint(const sm::Vector2& origin)
{
	m_spawnPoint = origin;
}

sm::Vector2 Wave::Group::GetSpawnPoint() const
{
	return m_spawnPoint;
}

int Wave::Group::GetEnemyTypeCount(const EnemyManagement::EnemyType& type)
{
	m_iterator = m_enemies.find(type);
	if (m_iterator != m_enemies.end())
		return m_iterator->second;
	return 0;
}

int Wave::Group::GetTotalNumberOfEnemies() const
{
	return m_totalEnemyCount;
}

void Wave::Group::ForEachEnemyType(std::function<void()>& func)
{
	for (auto& enemy : m_enemies)
	{
		func();
	}
}

std::vector<Wave::Group>& Wave::GetGroups()
{
	return m_groups;
}

// end class Group.


// class Wave.
Wave::Wave()
	: m_waveType(EnemyManagement::WaveType::Zone)
	  , m_timeLimit(60)
	  , m_distance(100.f)
	  , m_flankWidth(100.f)
{
}

void Wave::AddGroup(const Group& group)
{
	m_groups.emplace_back(group);
}

void Wave::SetDistance(const float& distance)
{
	m_distance = distance;
}

void Wave::SetFlankWidth(const float& width)
{
	m_flankWidth = width;
}

void Wave::SetTimeLimit(int time)
{
	m_timeLimit = time;
}

int Wave::GetTimeLimit() const
{
	return m_timeLimit;
}

EnemyManagement::WaveType Wave::GetWaveType() const
{
	return m_waveType;
}

float Wave::GetDistance() const
{
	return m_distance;
}

float Wave::GetFlankWidth() const
{
	return m_flankWidth;
}

// end class Wave.
