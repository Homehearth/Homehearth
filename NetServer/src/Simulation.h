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
	Currency m_currency;

	HeadlessScene* m_pLobbyScene;
	HeadlessScene* m_pGameScene;
	HeadlessScene* m_pCurrentScene;

	std::unordered_map<uint32_t, Entity> m_players;
	std::unordered_map<Entity, InputState> m_playerInputs;

	std::vector<Entity> m_addedEntities;
	std::vector<uint32_t> m_removedEntities;

	std::vector<Entity> m_updatedEntities;
	std::unordered_map<ecs::Component, std::vector<Entity>> m_updatedComponents;

	void InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask = UINT32_MAX) const;
	message<GameMsg> AllEntitiesMessage()const;

	uint32_t GetTick()const;
	
	void ScanForDisconnects();

	//Game play related
	Timer waveTimer;
	std::queue<Wave> waveQueue;
	std::queue<sm::Vector3> m_spawnPoints;
	
	void CreateWaves();
	
	void OnNetworkEntityCreate(entt::registry& reg, entt::entity entity);
	void OnNetworkEntityDestroy(entt::registry& reg, entt::entity entity);

	void OnNetworkEntityUpdated(entt::registry& reg, entt::entity entity);
	void OnComponentUpdated(Entity entity, ecs::Component component);


	std::vector<std::string> OpenFile(std::string filePath);
	void ConnectNodes(comp::Node* node1, comp::Node* node2);
	comp::Node* GetAINodeById(Vector2I& id);

	void BuildMapColliders(std::vector<dx::BoundingOrientedBox>* mapColliders);

public:
	Simulation(Server* pServer, HeadlessEngine* pEngine);
	virtual ~Simulation() = default;
	//bool AICreateNodes();
	bool AddNPC(uint32_t npcId);
	bool RemoveNPC(uint32_t npcId);


	// -1 will be defaulted to max value of unsigned 32 bit integer
	void Broadcast(message<GameMsg>& msg, uint32_t exclude = -1)const;
	void BroadcastUDP(message<GameMsg>& msg, uint32_t exclude = -1)const;

	bool AddPlayer(uint32_t playerID, const std::string& namePlate = "Noobie");
	bool RemovePlayer(uint32_t playerID);
	std::unordered_map<uint32_t, Entity>::iterator RemovePlayer(std::unordered_map<uint32_t, Entity>::iterator playerIterator);
	
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
	
	void SetLobbyScene();
	void SetGameScene();
	void ResetGameScene();
	
	void ResetPlayer(Entity e);

	void SendEntity(Entity e, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask = UINT32_MAX) const;
	void SendEntities(const std::vector<Entity>& entities, GameMsg msgID, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask = UINT32_MAX) const;

	void SendAllEntitiesToPlayer(uint32_t playerID) const;
	void SendRemoveAllEntitiesToPlayer(uint32_t playerID) const;
	void SendRemoveSingleEntity(Entity e) const;
	void SendRemoveSingleEntity(uint32_t networkID) const;

	void SendRemoveEntities(message<GameMsg>& msg) const;
	void SendRemoveEntities(const std::vector<uint32_t> entitiesNetIDs) const;

};