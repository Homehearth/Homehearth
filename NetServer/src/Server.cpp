#include "NetServerPCH.h"
#include "Server.h"

Server::Server()
{
	if (Start(4950))
	{
		std::cout << "Server started successfully!" << std::endl;
		T_CJOB(Server, Update);
	}
	else
	{
		std::cout << "Failed to start the server!" << std::endl;
	}
}

Server::~Server()
{

}

void Server::OnClientConnect(const std::string& ip, const uint16_t& port)
{
	std::cout << "Client connected from " << ip << ":" << std::to_string(port) << std::endl;
	Broadcast();
}

void Server::OnClientDisconnect()
{
	std::cout << "Client disconnected!" << std::endl;
}

void Server::OnMessageReceived(const SOCKET& socketId, const network::message<network::MessageType>& msg)
{
	std::cout << msg << std::endl;
}
