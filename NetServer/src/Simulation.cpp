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
	AddPlayer(hostID);
	return true;
}

// TODO ADD PLAYER FUNCTIONALITY
bool Simulation::AddPlayer(uint32_t playerID)
{
	LOG_INFO("Player with ID: %ld added to the game!", playerID);
	// Send new player id to other players
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddPlayer;
	msg << playerID << (uint32_t)1;
	for (auto con : m_connections)
	{
		m_server->SendToClient(con.second, msg);
	}

	// Send all player IDs to new Player
	message<GameMsg> msg1;
	msg1.header.id = GameMsg::Game_AddPlayer;
	for (auto con : m_connections)
	{
		msg1 << con.first;
	}
	msg1 << static_cast<uint32_t>(m_connections.size());
	m_server->SendToClient(m_server->GetConnection(playerID), msg1);

	m_connections[playerID] = m_server->GetConnection(playerID);

	return true;
}
