#include "NetServerPCH.h"
#include "Simulation.h"

Simulation::Simulation(Server* server)
	:m_server(server)
{
	this->m_gameID = 0;
}

bool Simulation::JoinLobby(uint32_t playerID, uint32_t gameID)
{
	// Send to client the message with the new game ID
	message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Accepted;
	msg << gameID;

	m_server->SendToClient(m_server->GetConnection(playerID), msg);

	// Add the players to the simulation on that specific client
	this->AddPlayer(playerID);

	return true;
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
	m_connections[playerID] = m_server->GetConnection(playerID);
	// Send new player id to other players
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddPlayer;
	msg << playerID << (uint32_t)1;
	Broadcast(msg);

	// Send all player IDs to new Player
	message<GameMsg> msg1;
	msg1.header.id = GameMsg::Game_AddPlayer;
	for (auto con : m_connections)
	{
		msg1 << con.first;
	}
	msg1 << static_cast<uint32_t>(m_connections.size());
	m_server->SendToClient(m_server->GetConnection(playerID), msg1);

	return true;
}

bool Simulation::RemovePlayer(uint32_t playerID)
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemovePlayer;
	msg << playerID;

	this->Broadcast(msg);

	return true;
}

void Simulation::UpdatePlayer(const uint32_t& playerID, message<GameMsg>& msg)
{
	msg << playerID;
	this->Broadcast(msg, playerID);
}

void Simulation::Broadcast(network::message<GameMsg>& msg, uint32_t exclude)
{
	auto it = m_connections.begin();

	while (it != m_connections.end())
	{
		if (m_server->GetConnection(it->first) != INVALID_SOCKET)
		{
			if (exclude != it->first)
			{
				m_server->SendToClient(it->second, msg);
			}
			it++;
		}
		else
		{
			uint32_t playerID = it->first;
			it = m_connections.erase(it);
 			this->RemovePlayer(playerID);
		}
	}
}
