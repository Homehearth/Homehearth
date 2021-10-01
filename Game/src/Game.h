#pragma once
#include <EnginePCH.h>
#include <Engine.h>
#include <Client.h>

class Game : public Engine
{
private:
	Client<network::GameMsg> m_client;
	// Maybe move this later
	std::unordered_map<uint64_t, entt::entity> players;
	uint64_t localPID;

	// Inherited via Engine
	virtual bool OnStartup() override;
	virtual bool OnUserUpdate(float deltaTime) override;

public:
	Game();
	virtual ~Game();

	void Start();
};