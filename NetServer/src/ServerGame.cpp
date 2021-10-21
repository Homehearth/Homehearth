#include "NetServerPCH.h"
#include "ServerGame.h"

using namespace std::placeholders;

ServerGame::ServerGame()
	:m_server(std::bind(&ServerGame::CheckIncoming, this, _1))
{
	m_nGameID = 0;
}

ServerGame::~ServerGame()
{

}

void ServerGame::InputThread()
{
	std::string input;
	while (1) {
		std::cin >> input;
		if (input == "/stop")
		{
			m_server.Stop();
			this->Shutdown();
			break;
		}
		else if (input == "/info")
		{
			LOG_INFO("INFO:")
				for (const auto& sim : m_simulations)
				{
					LOG_INFO("-------Simulation %u-------", sim.first);
					LOG_INFO("LOBBY SCENE:");
					LOG_INFO("\tEntity Count: %u", (unsigned int)sim.second->GetLobbyScene()->GetRegistry()->size());
					sim.second->GetLobbyScene()->ForEachComponent<comp::Network>([](Entity e, comp::Network& n)
						{
							LOG_INFO("\tEntity: %d", (entt::entity)e);
							LOG_INFO("\tNetwork id: %u", n.id);
						});

					LOG_INFO("GAME SCENE:");
					LOG_INFO("\tEntity Count: %u\n", (unsigned int)sim.second->GetGameScene()->GetRegistry()->size());
					sim.second->GetGameScene()->ForEachComponent<comp::Network>([](Entity e, comp::Network& n)
						{
							LOG_INFO("\tEntity: %d", (entt::entity)e);
							LOG_INFO("\tNetwork id: %u", n.id);
						});

				}
		}
	}
}

bool ServerGame::OnStartup()
{
	if (!m_server.Start(4950))
	{
		LOG_ERROR("Failed to start server");
		exit(0);
	}

	m_inputThread = std::thread(&ServerGame::InputThread, this);

	return true;
}

void ServerGame::OnShutdown()
{
	m_inputThread.join();
}


void ServerGame::UpdateNetwork(float deltaTime)
{
	// Check incoming messages
	this->m_server.Update();

	// Update the simulations
	for (auto it = m_simulations.begin(); it != m_simulations.end();)
	{
		if (it->second->IsEmpty())
		{
			it->second->Destroy();
			LOG_INFO("Destroyed empty lobby %d", it->first);
			it = m_simulations.erase(it);
		}
		else 
		{
			// Update the simulation
			it->second->Update(deltaTime);
			// Send the snapshot of the updated simulation to all clients in the sim
			it->second->SendSnapshot();
			it++;
		}
	}
}

void ServerGame::CheckIncoming(message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Server_GetPing:
	{
		uint32_t playerID;
		msg >> playerID;
		this->m_server.SendToClient(m_server.GetConnection(playerID), msg);
		LOG_INFO("Client on with ID: %ld is pinging server", playerID);
		break;
	}
	case GameMsg::Lobby_Create:
	{
		uint32_t playerID;
		msg >> playerID;
		if (this->CreateSimulation(playerID))
		{
			LOG_INFO("Created Game lobby!");
		}
		else
		{
			LOG_ERROR("Failed to create Lobby!");
		}
		break;
	}
	case GameMsg::Lobby_Join:
	{
		uint32_t gameID;
		msg >> gameID;
		uint32_t playerID;
		msg >> playerID;
		LOG_INFO("Player %d trying to join lobby %d", playerID, gameID);
		if (m_simulations.find(gameID) != m_simulations.end())
		{
			m_simulations[gameID]->JoinLobby(playerID, gameID);
		}
		else
		{
			message<GameMsg> invalidLobbyMsg;
			invalidLobbyMsg.header.id = GameMsg::Lobby_Invalid;
			invalidLobbyMsg << std::string("Player trying to join invalid Lobby");
			LOG_WARNING("Request denied: Player trying to join invalid Lobby");
			m_server.SendToClient(m_server.GetConnection(playerID), invalidLobbyMsg);
		}
		break;
	}
	case GameMsg::Lobby_Leave:
	{
		uint32_t gameID;
		msg >> gameID;
		uint32_t playerID;
		msg >> playerID;
		if (m_simulations.find(gameID) != m_simulations.end())
		{
			if (m_simulations[gameID]->LeaveLobby(playerID, gameID)) {

				// Send to client the message with the new game ID
				message<GameMsg> accMsg;
				accMsg.header.id = GameMsg::Lobby_AcceptedLeave;

				m_server.SendToClient(m_server.GetConnection(playerID), accMsg);
				break;
			}
		}

		message<GameMsg> invalidLobbyMsg;
		invalidLobbyMsg.header.id = GameMsg::Lobby_Invalid;
		invalidLobbyMsg << std::string("Player could not leave Lobby");
		LOG_WARNING("Request denied: Player could not leave Lobby");
		m_server.SendToClient(m_server.GetConnection(playerID), invalidLobbyMsg);

		break;
	}
	case GameMsg::Game_PlayerInput:
	{
		int8_t x;
		int8_t y;
		uint32_t playerID;
		uint32_t gameID;

		msg >> y >> x >> gameID >> playerID;

		if (m_simulations.find(gameID) != m_simulations.end())
		{
			m_simulations.at(gameID)->GetGameScene()->ForEachComponent<comp::Network, comp::Velocity>([=](comp::Network& net, comp::Velocity& vel)
				{
					if (net.id == playerID)
					{
						sm::Vector3 input(x, 0, y);
						input.Normalize(input);
						vel.vel = input * 10.f;
					}
				});
		}
		else
		{
			LOG_WARNING("Invalid GameID for player input message");
		}

		break;
	}
	}
}

bool ServerGame::CreateSimulation(uint32_t playerID)
{
	m_simulations[m_nGameID] = std::make_unique<Simulation>(&m_server, this);
	if (!m_simulations[m_nGameID]->Create(playerID, m_nGameID))
	{
		m_simulations.erase(m_nGameID);
		return false;
	}

	m_nGameID++;

	return true;
}
