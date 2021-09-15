#include "NetServerPCH.h"
#include "Server.h"

Server* Server::m_instance = nullptr;

Server::Server()
{
}

Server* Server::GetInstance()
{
	if (Server::m_instance == nullptr)
	{
		Server::m_instance = new Server;
	}

	return m_instance;
}

bool Server::IsRunning()
{
	return Server::GetInstance()->network::server_interface<network::MessageType>::IsRunning();
}

SOCKET Server::WaitForConnection()
{
	return Server::GetInstance()->network::server_interface<network::MessageType>::WaitForConnection();
}

Server::~Server()
{
}

void Server::OnClientConnect(std::string&& ip, const uint16_t& port)
{
	std::cout << "Client connected from " << ip << ":" << std::to_string(port) << std::endl;
	Broadcast();
}

void Server::OnClientDisconnect()
{
	std::cout << "Client disconnected!" << std::endl;
}

void Server::OnMessageReceived(const SOCKET& socketId, CHAR* buffer, DWORD bytesReceived)
{
	Send(socketId, buffer, bytesReceived);
}

void Server::OnClientValidated(const SOCKET& s)
{
	//using namespace network;
	//message<MessageType> msg = {};
	//msg.header.id = MessageType::Client_Accepted;
	//Send(s, msg);

	//std::cout << "Client has been validated!" << std::endl;
}

void Server::Destroy()
{
	delete Server::GetInstance();
}

void Server::Start(uint16_t port)
{
	Server::GetInstance()->network::server_interface<network::MessageType>::Start(4950);
}

void Server::Stop()
{
	Server::GetInstance()->network::server_interface<network::MessageType>::Stop();
}
