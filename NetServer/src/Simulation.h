#pragma once
#include "Server.h"
#include "HeadlessEngine.h"
#include "GridSystem.h"
#include "ServerSystems.h"
#include "Wave.h"

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
	GridSystem m_grid;

	HeadlessScene* m_pLobbyScene;
	HeadlessScene* m_pGameScene;
	HeadlessScene* m_pCurrentScene;
	
	std::unordered_map<uint32_t, Entity> m_players;
	std::unordered_map<Entity, InputState> m_playerInputs;

	std::vector<Entity> m_addedEntities;
	std::vector<uint32_t> m_removedEntities;


	void InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg)const;
	message<GameMsg> AllEntitiesMessage()const;

	uint32_t GetTick()const;
	
	// -1 will be defaulted to max value of unsigned 32 bit integer
	void Broadcast(message<GameMsg>& msg, uint32_t exclude = -1)const;
	void ScanForDisconnects();

	//Game play related
	Timer waveTimer;
	std::queue<Wave> waveQueue;
	void CreateWaves();
	
	void OnNetworkEntityCreate(entt::registry& reg, entt::entity entity);
	void OnNetworkEntityDestroy(entt::registry& reg, entt::entity entity);


public:
	Simulation(Server* pServer, HeadlessEngine* pEngine);
	virtual ~Simulation() = default;

	bool AddPlayer(uint32_t playerID, const std::string& namePlate = "Noobie");
	bool RemovePlayer(uint32_t playerID);
	std::unordered_map<uint32_t, Entity>::iterator RemovePlayer(std::unordered_map<uint32_t, Entity>::iterator playerIterator);

	bool AddEnemy();
	
	void SendSnapshot();
	bool JoinLobby(uint32_t playerID, uint32_t gameID, const std::string& namePlate = "Noobie");
	bool LeaveLobby(uint32_t playerID, uint32_t gameID);
	// Update the visuals to the player.
	void UpdateLobby();

	bool Create(uint32_t playerID, uint32_t gameID, std::vector<dx::BoundingOrientedBox>* mapColliders, const std::string& namePlate = "Noobie");
	void Destroy();

	// Updates the lobby.
	void ReadyCheck(const uint32_t& playerID);

	bool IsEmpty() const;

	void NextTick();

	void Update(float dt);
	void UpdateInput(InputState state, uint32_t playerID);
	
	HeadlessScene* GetLobbyScene() const;
	HeadlessScene* GetGameScene() const;

	void SendEntity(Entity e, uint32_t exclude = -1)const;
	void SendEntities(const std::vector<Entity>& entities)const;

	void SendAllEntitiesToPlayer(uint32_t playerID)const;
	void SendRemoveAllEntitiesToPlayer(uint32_t playerID)const;
	void SendRemoveSingleEntity(Entity e)const;
	void SendRemoveSingleEntity(uint32_t networkID)const;

	void SendRemoveEntities(message<GameMsg>& msg)const;
	void SendRemoveEntities(const std::vector<uint32_t> entitiesNetIDs)const;

	uint32_t GetUniqueID();
};