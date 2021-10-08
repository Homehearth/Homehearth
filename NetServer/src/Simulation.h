#pragma once
#include "Server.h"
/* 
		Simulation defines each ongoing simulation from the perspective of the server
		gameID identifies the simulation which each player has to give the server to keep track
		players are identified through and unique key which is sent by the server at validation
*/

class Simulation
{
private:
	Server* m_server;
	uint32_t m_gameID;
	/*
	TODO : Add scenes to incorporate ECS and a structure to update the simulation
	*/
	//Scene Lobby;
	//Scene Game;
	std::unordered_map<uint32_t, entt::entity> m_players;
	std::unordered_map<uint32_t, SOCKET> m_connections;

	bool AddPlayer(uint32_t playerID);

public:
	Simulation(Server* server);
	virtual ~Simulation() = default;

	bool JoinLobby(uint32_t playerID, uint32_t gameID);
	bool CreateLobby(uint32_t playerID, uint32_t gameID);
	void UpdatePlayer(const uint32_t& playerID, message<GameMsg>& msg);

	// -1 will be defaulted to max value of unsigned 32 bit integer
	void Broadcast(network::message<GameMsg>& msg, uint32_t exclude = -1);
};