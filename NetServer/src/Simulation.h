#pragma once
#include "Server.h"
#include "HeadlessEngine.h"
#include "GridSystem.h"
#include "PathFinderManager.h"
#include "ServerSystems.h"
#include "Wave.h"
#include "AIBehaviors.h"
#include "Lobby.h"
#include "IShop.h"
#include "QuadTree.h"
#include "Cycler.h"
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
	Lobby m_lobby;
	IShop m_shop;
	std::unique_ptr<QuadTree> qt;
	std::unique_ptr<QuadTree> qtDynamic;
	sm::Vector3 TL = { 220.f, 0, -353.f };
	sm::Vector3 TR = { 197.f, 0, -325.f };
	sm::Vector3 BR = { 222.f, 0, -300.f };
	sm::Vector3 BL = { 247.f, 0, -325.f };

	HeadlessScene* m_pLobbyScene;
	HeadlessScene* m_pGameScene;
	HeadlessScene* m_pGameOverScene;
	HeadlessScene* m_pCurrentScene;

	std::vector<Entity> m_addedEntities;
	std::vector<uint32_t> m_removedEntities;
	std::vector<Entity> m_updatedEntities;

	std::unordered_map<ecs::Component, std::vector<Entity>> m_updatedComponents;

	int currentRound;

	void InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask = UINT32_MAX) const;

	uint32_t GetTick()const;

	//Game play related
	Timer waveTimer;
	std::queue<Wave> waveQueue;
	std::queue<sm::Vector3> m_spawnPoints;
	
	void OnNetworkEntityCreate(entt::registry& reg, entt::entity entity);
	void OnNetworkEntityDestroy(entt::registry& reg, entt::entity entity);

	void OnNetworkEntityUpdated(entt::registry& reg, entt::entity entity);
	void OnComponentUpdated(Entity entity, ecs::Component component);

	void BuildMapColliders(std::vector<dx::BoundingOrientedBox>* mapColliders);

public:
	Cycler m_timeCycler;
	Simulation(Server* pServer, HeadlessEngine* pEngine);
	virtual ~Simulation() = default;
	
	void SendSnapshot();
	void JoinLobby(uint32_t gameID, uint32_t playerID, const std::string& name = "Noobie");
	void LeaveLobby(uint32_t playerID);

	bool Create(uint32_t gameID, std::vector<dx::BoundingOrientedBox>* mapColliders);
	void Destroy();

	void NextTick();

	void Update(float dt);
	void UpdateInput(InputState state, uint32_t playerID);

	HeadlessScene* GetLobbyScene() const;
	HeadlessScene* GetGameOverScene() const;
	HeadlessScene* GetGameScene() const;

	GridSystem& GetGrid();
	Currency& GetCurrency();

	void SetLobbyScene();
	void SetGameOver();
	void SetGameScene();
	void ResetGameScene();
	
	void ResetPlayer(Entity player);

	void SendEntities(const std::vector<Entity>& entities, GameMsg msgID, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask = UINT32_MAX);
	void SendAllEntitiesToPlayer(uint32_t playerID) const;

	void SendRemoveAllEntitiesToPlayer(uint32_t playerID) const;
	void SendRemoveEntities(const std::vector<uint32_t> entitiesNetIDs);

	void SendMsg(uint32_t playerID, message<GameMsg>&msg)const;
	void SendMsgUDP(uint32_t playerID, message<GameMsg>& msg)const;

	bool IsPlayerConnected(uint32_t playerID);
	bool IsEmpty()const;
	void ReadyCheck(uint32_t playerID);
	void Broadcast(message<GameMsg>& msg, uint32_t exclude = -1)const;
	void BroadcastUDP(message<GameMsg>& msg, uint32_t exclude = -1)const;

	Entity GetPlayer(uint32_t playerID)const;

	void UseShop(const ShopItem& item, const uint32_t& player);
	void UpgradeDefence(const uint32_t& id);
};