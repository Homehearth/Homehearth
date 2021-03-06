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
#include "HouseManager.h"
#include "SpreeHandler.h"
/*
		Simulation defines each ongoing simulation from the perspective of the server
		gameID identifies the simulation which each player has to give the server to keep track
		players are identified through and unique key which is sent by the server at validation
*/


class Simulation
{
private:
	static const uint8_t PACKET_CHUNK_SIZE = 8;
	Server* m_pServer;
	HeadlessEngine* m_pEngine;
	uint32_t m_gameID;
	uint32_t m_tick;
	GridSystem m_grid;
	Currency m_currency;
	SpreeHandler m_spreeHandler;
	std::unique_ptr<QuadTree> qt;
	std::unique_ptr<QuadTree> qtDynamic;
	sm::Vector3 TL = { 220.f, 1.25f, -353.f };
	sm::Vector3 TR = { 197.f, 1.25f, -325.f };
	sm::Vector3 BR = { 222.f, 1.25f, -300.f };
	sm::Vector3 BL = { 247.f, 1.25f, -325.f };

	HeadlessScene* m_pLobbyScene;
	HeadlessScene* m_pGameScene;
	HeadlessScene* m_pCurrentScene;

	std::vector<Entity> m_addedEntities;
	std::vector<uint32_t> m_removedEntities;
	std::vector<Entity> m_updatedEntities;

	std::unique_ptr<Blackboard> blackboard;

	std::unordered_map<ecs::Component, std::vector<Entity>> m_updatedComponents;

	void InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask = UINT32_MAX) const;

	std::queue<sm::Vector3> m_spawnPoints;
	HouseManager houseManager;
	void SendRemoveEntities(const std::vector<uint32_t> entitiesNetIDs);

	void SendSnapshot();
	void SendAddedEntities();
	void OnNetworkEntityCreate(entt::registry& reg, entt::entity entity);
	void OnNetworkEntityDestroy(entt::registry& reg, entt::entity entity);

	void OnNetworkEntityUpdated(entt::registry& reg, entt::entity entity);
	void OnComponentUpdated(Entity entity, ecs::Component component);

	void BuildMapColliders(std::vector<dx::BoundingOrientedBox>* mapColliders);
public:
	Cycler m_timeCycler;
	std::queue<Wave> waveQueue;
	uint32_t currentRound;
	Lobby m_lobby;
	uint32_t m_wavesSurvived;
	IShop m_shop;
	Entity m_rainEntity;

	Simulation(Server* pServer, HeadlessEngine* pEngine);
	virtual ~Simulation() = default;

	void JoinLobby(uint32_t gameID, uint32_t playerID, const std::string& name = "Noobie");
	void LeaveLobby(uint32_t playerID);

	bool Create(uint32_t gameID, std::vector<dx::BoundingOrientedBox>* mapColliders, std::unordered_map<std::string, comp::OrientedBoxCollider>* houseColliders);
	void Destroy();

	void Update(float dt);
	void UpdateInput(InputState state, uint32_t playerID);

	HeadlessScene* GetLobbyScene() const;
	HeadlessScene* GetGameScene() const;
	void SetScene(HeadlessScene* scene);

	GridSystem& GetGrid();
	Currency& GetCurrency();

	void SetLobbyScene();
	void SetGameScene();
	void ResetGameScene();

	void ResetPlayer(Entity player);

	void SendEntities(const std::vector<Entity>& entities, GameMsg msgID, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask = UINT32_MAX);
	void SendAllEntitiesToPlayer(uint32_t playerID) const;

	void SendRemoveAllEntitiesToPlayer(uint32_t playerID) const;
	void SendRemovedEntities();

	void SendMsg(uint32_t playerID, message<GameMsg>& msg)const;
	void SendMsgUDP(uint32_t playerID, message<GameMsg>& msg)const;

	bool IsPlayerConnected(uint32_t playerID);
	bool IsEmpty()const;
	void ReadyCheck(uint32_t playerID);
	void Broadcast(message<GameMsg>& msg, uint32_t exclude = -1)const;
	void BroadcastUDP(message<GameMsg>& msg, uint32_t exclude = -1)const;

	Entity GetPlayer(uint32_t playerID)const;
	Blackboard* GetBlackboard();

	void UpgradeDefence(const uint32_t& id);
	void ClearOutgoing();
};
