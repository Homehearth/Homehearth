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

void ServerGame::Run() 
{

	std::thread t(&ServerGame::InputThread, this);

	HeadlessEngine::Run();

	t.join();
}

bool ServerGame::OnStartup()
{
	if (!m_server.Start(4950))
	{
		LOG_ERROR("Failed to start server");
		exit(0);
	}


	return true;
}

void ServerGame::Start()
{
	Startup();
}
