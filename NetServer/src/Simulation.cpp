#include "NetServerPCH.h"
#include "Simulation.h"

Simulation::Simulation(Server* server)
	:m_server(server)
{
	this->m_gameID = 0;
}

bool Simulation::CreateLobby(uint32_t uniqueGameID, uint32_t hostID)
{
	this->m_gameID = uniqueGameID;
	m_connections[hostID] = m_server->GetConnection(hostID);

	return true;
}

// TODO ADD PLAYER FUNCTIONALITY
bool Simulation::AddPlayer(uint32_t playerID)
{
	LOG_INFO("Player with ID: %ld added to the game!", playerID);
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddPlayer;
	msg << playerID;
	for (auto con : m_connections)
	{
		m_server->SendToClient(con.second, msg);
	}

	m_connections[playerID] = m_server->GetConnection(playerID);

	return true;
}
