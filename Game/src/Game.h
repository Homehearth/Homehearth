#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <GridSystem.h>
#include "ModelIdentifier.h"
#include "ParticleSystem.h"

class Game : public Engine
{
private:
	std::chrono::system_clock::time_point m_timeThen;
	std::unordered_map<uint32_t, Entity> m_gameEntities;

	GridSystem m_grid;
	float m_elapsedCycleTime = 0;
	uint32_t m_waveTimer = 0;
	uint32_t m_money;
	ParticleSystem m_particles;
	Mode m_mode = Mode::PLAY_MODE;
	Cycle m_serverCycle = Cycle::DAY;

	Entity m_mapEntity;

	InputState m_inputState;
	std::vector<InputState> m_savedInputs;


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

	void UpdateInput();

public:
	Client m_client;
	uint32_t m_localPID;
	uint32_t m_gameID;
	uint32_t m_spectatingID;
	std::string m_playerName;
	std::unordered_map<ModelID, std::vector<Entity>> m_models;
	std::vector<std::pair<ModelID, dx::BoundingSphere>> m_LOSColliders;
	std::unordered_map<uint32_t, Entity> m_players;


	Game();
	virtual ~Game();
	void JoinLobby(uint32_t lobbyID);
	void CreateLobby();
	const Mode& GetCurrentMode() const;
	void SetMode(const Mode& mode);
	const uint32_t& GetMoney() const;
	
	void SendStartGame();
	void SendSelectedClass(comp::Player::Class classType);

	Entity& GetLocalPlayer();

	ParticleSystem* GetParticleSystem();
	void UseShop(const ShopItem& whatToBuy);
	void UpgradeDefence(const uint32_t& id);
};