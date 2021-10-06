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
			Shutdown();
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
	case GameMsg::Game_MovePlayer:
	{
		LOG_INFO("Moving player!");
		break;
	}
	case GameMsg::Client_CreateLobby:
	{
		LOG_INFO("Creating a game lobby!");
		uint32_t uniquePID;
		msg >> uniquePID;
		this->CreateSimulation(uniquePID);
		break;
	}
	case GameMsg::Client_JoinLobby:
	{
		LOG_INFO("Joining Game lobby!");
		uint32_t lobbyID;
		msg >> lobbyID;
		uint32_t playerID;
		msg >> playerID;
		games[lobbyID]->AddPlayer(playerID);
		break;
	}
	}
}

bool ServerGame::CreateSimulation(uint32_t lobbyLeaderID)
{
	games[m_nGameID] = std::make_unique<Simulation>(&m_server);
	if (!games[m_nGameID]->CreateLobby(m_nGameID, lobbyLeaderID))
	{
		games.erase(m_nGameID);
		return false;
	}
	m_nGameID++;

	return true;
}
