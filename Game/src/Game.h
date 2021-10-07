#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <DemoScene.h>

class Game : public Engine
{
private:
	std::chrono::system_clock::time_point m_timeThen;
	Client m_client;
	// Maybe move this later
	std::unordered_map<uint32_t, Entity> m_players;
	uint32_t m_localPID;
	uint32_t m_gameID;

	std::unique_ptr<DemoScene> m_demoScene;

	// Inherited via Engine
	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;

	// User defined function to check messages which must comply with the function pointer arguments from Client
	void CheckIncoming(message<GameMsg>& msg);
	void PingServer();
	void JoinLobby(uint32_t lobbyID);

public:
	Game();
	virtual ~Game();
};