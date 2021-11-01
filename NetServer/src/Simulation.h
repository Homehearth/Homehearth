#pragma once
#include "Server.h"
#include "HeadlessEngine.h"
#include "ServerSystems.h"


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
	std::unordered_map<Entity, InputState> m_playerInputs;


	void InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg)const;
	message<GameMsg> AllEntitiesMessage()const;
	message<GameMsg> SingleEntityMessage(Entity entity)const;

	uint32_t GetTick()const;

	// -1 will be defaulted to max value of unsigned 32 bit integer
	void Broadcast(message<GameMsg>& msg, uint32_t exclude = -1);
	void ScanForDisconnects();

	std::vector<std::string> OpenFile(std::string filePath);
	void ConnectNodes(comp::Node* node1, comp::Node* node2);
	comp::Node* GetAINodeById(uint32_t id);

public:
	Simulation(Server* pServer, HeadlessEngine* pEngine);
	virtual ~Simulation() = default;
	bool AICreateNodes();
	bool AIAStarSearch();
	bool AddPlayer(uint32_t playerID);
	bool RemovePlayer(uint32_t playerID);
	bool AddNPC(uint32_t npcId);
	bool RemoveNPC(uint32_t npcId);
	bool AddEnemy();

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
	void UpdateInput(InputState state, uint32_t playerID);

	HeadlessScene* GetLobbyScene() const;
	HeadlessScene* GetGameScene() const;
};