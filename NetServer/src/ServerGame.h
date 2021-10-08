#pragma once
#pragma once

#include <HeadlessEngine.h>
#include "Simulation.h"


class ServerGame : public HeadlessEngine 
{
private:
	Server m_server;
	std::thread m_inputThread;

	uint32_t m_nGameID;
	std::unordered_map<uint32_t, std::unique_ptr<Simulation>> games;

	// User defined function to check messages which must comply with the function pointer arguments from Server
	void CheckIncoming(message<GameMsg>& msg);

	bool CreateSimulation(uint32_t playerID);

public:
	ServerGame();
	virtual ~ServerGame();

	void InputThread();


	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;
};

