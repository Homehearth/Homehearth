#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <DemoScene.h>


class Game : public Engine
{
private:
	std::chrono::system_clock::time_point m_timeThen;
	Client m_client;
	uint32_t m_localPID;
	uint32_t m_gameID;
	std::string* m_ipBuffer = nullptr;
	std::string* m_lobbyBuffer = nullptr;
	std::string* m_portBuffer = nullptr;

	bool m_isLeavingLobby;

	// Inherited via Engine
	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;
	virtual void UpdateNetwork(float deltaTime) override;

	// User defined function to check messages which must comply with the function pointer arguments from Client
	void CheckIncoming(message<GameMsg>& msg);
	void PingServer();
	void JoinLobby(uint32_t lobbyID);
	void CreateLobby();
	void OnClientDisconnect();
	
	Entity CreateEntityFromMessage(message<GameMsg>& msg);

public:
	Game();
	virtual ~Game();

	// Inherited via Engine
};