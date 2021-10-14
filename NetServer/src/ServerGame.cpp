#include "NetServerPCH.h"
#include "ServerGame.h"

using namespace std::placeholders;

ServerGame::ServerGame()
	:m_server(std::bind(&ServerGame::CheckIncoming, this, _1))
{
	m_nGameID = 10000;
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
				LOG_INFO("LOBBY SCENE:");
				LOG_INFO("Entity Count: %u", (unsigned int)sim.second->GetLobbyScene()->GetRegistry()->size());
				LOG_INFO("GAME SCENE:");
				LOG_INFO("Entity Count: %u\n", (unsigned int)sim.second->GetGameScene()->GetRegistry()->size());

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

void ServerGame::OnUserUpdate(float deltaTime)
{
	this->m_server.Update();

	for (const auto& sim : m_simulations)
	{
		sim.second->Update(deltaTime);
	}

}

void ServerGame::OnShutdown()
{
	m_inputThread.join();
}

void ServerGame::UpdateNetwork(float deltaTime)
{
	for (const auto& sim : m_simulations)
	{
		sim.second->SendSnapshot();
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
			LOG_WARNING("Request denied: Player trying to join invalid Lobby");
			m_server.SendToClient(m_server.GetConnection(playerID), invalidLobbyMsg);
		}
		break;
	}
	case GameMsg::Game_MovePlayer:
	{
		int8_t x;
		int8_t y;
		uint32_t playerID;
		uint32_t gameID;

		msg >> y >> x >> gameID >> playerID;

		m_simulations.at(gameID)->GetGameScene()->ForEachComponent<comp::Network, comp::Velocity>([=](comp::Network& net, comp::Velocity& vel) 
			{
				if (net.id == playerID)
				{
					vel.vel	= sm::Vector3(x, 0, y) * 10.f;
				}
			});
		
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
