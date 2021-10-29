#pragma once
#include "Server.h"
#include "HeadlessEngine.h"
#include "GridSystem.h"

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
	
	HeadlessScene* m_pLobbyScene;
	HeadlessScene* m_pGameScene;
	HeadlessScene* m_pCurrentScene;
	
	std::unordered_map<uint32_t, entt::entity> m_players;
	std::unordered_map<uint32_t, SOCKET> m_connections;

	GridSystem m_grid;

	void InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg);
	message<GameMsg> AllEntitiesMessage();
	message<GameMsg> SingleEntityMessage(Entity entity);

public:
	Simulation(Server* pServer, HeadlessEngine* pEngine);
	virtual ~Simulation() = default;

	bool AddPlayer(uint32_t playerID);
	bool RemovePlayer(uint32_t playerID);
	
	bool JoinLobby(uint32_t playerID, uint32_t gameID);
	bool LeaveLobby(uint32_t playerID, uint32_t gameID);

	bool Create(uint32_t playerID, uint32_t gameID);
	void Destroy();

	bool IsEmpty() const;

	void SendSnapshot();

	void Update(float dt);

	// -1 will be defaulted to max value of unsigned 32 bit integer
	void Broadcast(network::message<GameMsg>& msg, uint32_t exclude = -1);

	HeadlessScene* GetLobbyScene() const;
	HeadlessScene* GetGameScene() const;

};