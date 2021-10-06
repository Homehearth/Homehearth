#pragma once

#include <HeadlessEngine.h>
#include "Simulation.h"


class ServerGame : public HeadlessEngine 
{
private:
	Server m_server;
	uint32_t m_nGameID;
	std::thread inputThread;
	std::unordered_map<uint32_t, std::unique_ptr<Simulation>> games;

	// User defined function to check messages which must comply with the function pointer arguments from Server
	void CheckIncoming(message<GameMsg>& msg);

	bool CreateSimulation(uint32_t lobbyLeaderID);

public:
	ServerGame();
	virtual ~ServerGame();

	void InputThread();

	virtual bool OnStartup();
	void Start();

	// Inherited via HeadlessEngine
	virtual bool OnUserUpdate(float deltaTime) override;
};

