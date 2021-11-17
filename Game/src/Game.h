#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <GridSystem.h>
#include "Predictor.h"
#include "ParticleSystem.h"

class Game : public Engine
{
private:
	std::chrono::system_clock::time_point m_timeThen;
	std::vector<comp::Transform> predictedPositions;
	std::unordered_map<uint32_t, Entity> m_players;
	std::unordered_map<uint32_t, Entity> m_gameEntities;
	std::vector<dx::BoundingOrientedBox> m_LOSColliders;

	GridSystem m_grid;
	uint32_t m_waveTimer;
	ParticleSystem m_particles;


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
	void LoadAllAssets();
	bool LoadMapColliders(const std::string& filename);

public:
	Client m_client;
	Predictor m_predictor;
	uint32_t m_localPID;
	uint32_t m_gameID;
	std::string m_playerName;

	float m_masterVolume = 5.0f;

	Game();
	virtual ~Game();
	void JoinLobby(uint32_t lobbyID);
	void CreateLobby();
	void SendStartGame();
	Entity& GetLocalPlayer();

	ParticleSystem* GetParticleSystem();
};