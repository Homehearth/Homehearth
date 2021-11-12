#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <GridSystem.h>
#include "ModelIdentifier.h"

class Game : public Engine
{
private:
	std::chrono::system_clock::time_point m_timeThen;
	std::vector<comp::Transform> predictedPositions;
	std::unordered_map<uint32_t, Entity> m_gameEntities;
	std::vector<dx::BoundingSphere> m_LOSColliders;
	std::unordered_map<uint32_t, Entity> m_players;

	GridSystem m_grid;
	uint32_t m_waveTimer;

	Entity m_mapEntity;

	float m_predictionThreshhold;

	InputState m_inputState;

	// Inherited via Engine
	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;
	virtual void UpdateNetwork(float deltaTime) override;

	// User defined function to check messages which must comply with the function pointer arguments from Client
	void CheckIncoming(message<GameMsg>& msg);
	void PingServer();
	void OnClientDisconnect();
	
	void UpdateEntityFromMessage(Entity entity, message<GameMsg>& msg);
	void UpdatePredictorFromMessage(Entity entity, message<GameMsg>& msg, const uint32_t& id);

	void UpdateInput();

public:
	Client m_client;
	uint32_t m_localPID;
	uint32_t m_gameID;
	std::string m_playerName;
	std::unordered_map<ModelID, std::vector<comp::Renderable>> m_models;

	float m_masterVolume = 5.0f;

	Game();
	virtual ~Game();
	void JoinLobby(uint32_t lobbyID);
	void CreateLobby();
	
	void SendStartGame();
	void SendSelectedClass(comp::Player::Class classType);

	Entity& GetLocalPlayer();
};