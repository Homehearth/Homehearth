#include "NetServerPCH.h"
#include "ServerGame.h"

ServerGame::ServerGame() 
{

}

bool ServerGame::OnStartup()
{
	if (!m_server.Start(4950))
	{
		//LOG_ERROR("");
		exit(0);
	}
	
	while (m_server.IsRunning())
	{

	}
	
	return true;
}

void ServerGame::Start()
{
	//Startup();
}
