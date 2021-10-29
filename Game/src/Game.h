#pragma once
#include <EnginePCH.h>
#include <Engine.h>

class Game : public Engine
{
private:
	std::chrono::system_clock::time_point m_timeThen;

	std::string* m_ipBuffer = nullptr;
	std::string* m_lobbyBuffer = nullptr;
	std::string* m_portBuffer = nullptr;
	std::vector<comp::Transform> predictedPositions;

	bool m_isLeavingLobby;
	comp::Transform test;
	float m_predictionThreshhold;

	// Inherited via Engine
	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;
	virtual void UpdateNetwork(float deltaTime) override;

	// User defined function to check messages which must comply with the function pointer arguments from Client
	void CheckIncoming(message<GameMsg>& msg);
	void PingServer();
	void OnClientDisconnect();
	Entity CreateEntityFromMessage(message<GameMsg>& msg);

public:
	Client m_client;
	uint32_t m_localPID;
	uint32_t m_gameID;
	Game();
	virtual ~Game();
	void JoinLobby(uint32_t lobbyID);
	void CreateLobby();
	void SendStartGame();
};