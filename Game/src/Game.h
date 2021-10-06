#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <Client.h>

class Game : public Engine
{
private:
	Client m_client;
	// Maybe move this later
	std::unordered_map<uint64_t, entt::entity> players;
	uint32_t localPID;

	// Inherited via Engine
	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;


public:
	Game();
	virtual ~Game();
};