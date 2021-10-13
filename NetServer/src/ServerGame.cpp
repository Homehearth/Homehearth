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
}

void ServerGame::OnShutdown()
{
	m_inputThread.join();
}

void ServerGame::UpdateNetwork(float deltaTime)
{
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
		if (games.find(gameID) != games.end())
		{
			games[gameID]->JoinLobby(playerID, gameID);
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
	case GameMsg::Game_Update:
	{
		uint32_t gameID;
		uint32_t playerID;
		msg >> gameID >> playerID;
		if (games.find(gameID) != games.end())
		{
			games[gameID]->UpdatePlayer(playerID, msg);
		}
		break;
	}
	case GameMsg::Game_MovePlayer:
	{
		int x;
		int y;

		msg >> y >> x;

		if (x || y)
		{
			LOG_INFO("Player is moving in X: %d Y: %d", x, y);
		}
		break;
	}
	}
}

bool ServerGame::CreateSimulation(uint32_t playerID)
{
	games[m_nGameID] = std::make_unique<Simulation>(&m_server);
	if (!games[m_nGameID]->CreateLobby(playerID, m_nGameID))
	{
		games.erase(m_nGameID);
		return false;
	}

	m_nGameID++;

	return true;
}
