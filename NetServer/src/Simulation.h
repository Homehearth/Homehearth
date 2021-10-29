#pragma once
#include "Server.h"
#include "HeadlessEngine.h"

constexpr int MAX_PLAYER_PER_LOBBY = 2;

/* 
		Simulation defines each ongoing simulation from the perspective of the server
		gameID identifies the simulation which each player has to give the server to keep track
		players are identified through and unique key which is sent by the server at validation
*/

class Simulation
{
private:
	Server* m_pServer;
	HeadlessEngine* m_pEngine;
	uint32_t m_gameID;
	uint32_t m_tick;

	struct pDecision_t
	{
		uint32_t playerID = -1;
		bool isWantToStart = false;
	}m_playerDecisions[MAX_PLAYER_PER_LOBBY];
	
	HeadlessScene* m_pLobbyScene;
	HeadlessScene* m_pGameScene;
	HeadlessScene* m_pCurrentScene;
	
	std::unordered_map<uint32_t, Entity> m_players;

	void InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg)const;
	message<GameMsg> AllEntitiesMessage()const;

	uint32_t GetTick()const;
	bool AddPlayer(uint32_t playerID);
	bool RemovePlayer(uint32_t playerID);

	// -1 will be defaulted to max value of unsigned 32 bit integer
	void Broadcast(message<GameMsg>& msg, uint32_t exclude = -1)const;
	void ScanForDisconnects();

public:
	Simulation(Server* pServer, HeadlessEngine* pEngine);
	virtual ~Simulation() = default;

	void SendSnapshot();
	bool JoinLobby(uint32_t playerID, uint32_t gameID);
	bool LeaveLobby(uint32_t playerID, uint32_t gameID);

	bool Create(uint32_t playerID, uint32_t gameID);
	void Destroy();

	// Updates the lobby.
	void UpdateLobby(const uint32_t& playerID);

	bool IsEmpty() const;

	void NextTick();

	void Update(float dt);
	HeadlessScene* GetLobbyScene() const;
	HeadlessScene* GetGameScene() const;

	void SendEntity(Entity e)const;
	void SendAllEntitiesToPlayer(uint32_t playerID)const;
	void SendRemoveAllEntitiesToPlayer(uint32_t playerID)const;
	void SendRemoveSingleEntity(Entity e)const;
	void SendRemoveEntities(message<GameMsg>& msg)const;

	uint32_t GetUniqueID();
};