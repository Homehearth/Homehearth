#include "NetServerPCH.h"
#include "Server.h"

Server::Server()
{
	Start(4950);
	T_CJOB(Server, Update);
}

Server::~Server()
{

}

void Server::OnClientConnect(const std::string& ip, const uint16_t& port)
{
	std::cout << "Client connected from " << ip << ":" << std::to_string(port) << std::endl;
}

void Server::OnClientDisconnect()
{
	std::cout << "Client disconnected!" << std::endl;
}

void Server::OnMessageReceived(const SOCKET& socketId, const network::message<network::MessageType>& msg)
{
	std::cout << msg << std::endl;
}
