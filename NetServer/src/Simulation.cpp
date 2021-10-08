#include "NetServerPCH.h"
#include "Simulation.h"

Simulation::Simulation(Server* server)
	:m_server(server)
{
	this->m_gameID = 0;
}

bool Simulation::CreateLobby(uint32_t playerID, uint32_t gameID)
{
	this->m_gameID = gameID;
	AddPlayer(playerID);

	message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Accepted;
	msg << m_gameID;

	m_server->SendToClient(m_connections[playerID], msg);

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
	Broadcast(msg, playerID);

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

void Simulation::UpdatePlayer(uint32_t playerID, const comp::Transform& transform)
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_Update;
	msg << transform << playerID;
	Broadcast(msg, playerID);
}

void Simulation::Broadcast(network::message<GameMsg>& msg, uint32_t exclude)
{
	for (auto con : m_connections)
	{
		if (exclude != con.first)
		{
			m_server->SendToClient(m_server->GetConnection(con.first), msg);
		}
	}
}