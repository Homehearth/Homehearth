#pragma once
#include <functional>

class Simulation;

class Lobby
{
private:
	Simulation* m_simRef;
	bool m_isActive;

	std::unordered_map<uint32_t, Entity>::iterator RemovePlayer(std::unordered_map<uint32_t, Entity>::iterator playerIterator);
	void CreatePlayerEntity(uint32_t playerID, const std::string& name);

	std::priority_queue<comp::Player::PlayerType, std::vector<comp::Player::PlayerType>, std::greater<comp::Player::PlayerType>> m_playerTypes;

public:
	Lobby();
	~Lobby();

	std::unordered_map<uint32_t, Entity> m_players;
	void Init(Simulation* sim);
	void AddPlayer(uint32_t gameID, uint32_t playerID, const std::string& name);
	void Update();
	bool IsActive()const;
	void SetActive(bool active);
	void ScanForDisconnects();
	Entity GetPlayer(uint32_t playerID)const;
	bool ReadyCheck(uint32_t playerID);
	bool IsEmpty()const;
	void RemovePlayer(uint32_t playerID);
	void Clear();
};