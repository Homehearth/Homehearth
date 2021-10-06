#include "NetServerPCH.h"
#include "ServerGame.h"

ServerGame::ServerGame() 
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
	this->m_server.Update(1);

}

void ServerGame::OnShutdown()
{
	m_inputThread.join();
}
