#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <Client.h>

class Game : public Engine
{
private:
	std::chrono::system_clock::time_point timeThen;
	Client m_client;
	// Maybe move this later
	std::unordered_map<uint64_t, entt::entity> players;
	uint32_t localPID;

	// Inherited via Engine
	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;


	// User defined function to check messages which must comply with the function pointer arguments from Client
	void CheckIncoming(message<GameMsg>& msg);
	void PingServer();

public:
	Game();
	virtual ~Game();
};