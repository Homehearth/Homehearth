#pragma once
#include <HeadlessEngine.h>
#include "Simulation.h"


class ServerGame : public HeadlessEngine
{
private:
	Server m_server;
	std::thread m_inputThread;
	static const int MAX_ACTIVE_GAMES = 5;

	uint32_t m_nGameID;
	std::unordered_map<uint32_t, std::unique_ptr<Simulation>> m_simulations;
	std::vector<dx::BoundingOrientedBox> m_mapColliders;
	std::unordered_map<std::string,comp::OrientedBoxCollider> m_houseColliders;

	// Load in all the colliders from a file
	bool LoadMapColliders(const std::string& filename);
	bool LoadHouseColliders(const std::string& filename);
	// User defined function to check messages which must comply with the function pointer arguments from Server
	void CheckIncoming(message<GameMsg>& msg);

	uint32_t CreateSimulation();
	
	// Inherited via HeadlessEngine
	virtual void UpdateNetwork(float deltaTime) override;

public:
	ServerGame();
	virtual ~ServerGame();

	void InputThread();



	virtual bool OnStartup() override;
	virtual void OnShutdown() override;
};

