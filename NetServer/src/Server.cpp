#include "NetServerPCH.h"
#include "Server.h"

namespace network
{
	Server::Server()
	{
	}

	Server::~Server()
	{
	}

	void Server::OnClientConnect(std::string&& ip, const uint16_t& port)
	{
		EnterCriticalSection(&lock);
		LOG_INFO("Client connected from %s:%d", ip.c_str(), port);
		LeaveCriticalSection(&lock);
	}

	void Server::OnClientDisconnect()
	{
		EnterCriticalSection(&lock);
		LOG_INFO("Client disconnected!");
		LeaveCriticalSection(&lock);
	}

	void Server::OnMessageReceived(SOCKET_INFORMATION*& SI, message<MessageType>& msg)
	{
		switch (msg.header.id)
		{
		case MessageType::Server_GetPing:
		{
			message<MessageType> msg = {};
			msg.header.id = MessageType::Server_GetPing;
			this->SendToClient(SI, msg);
			EnterCriticalSection(&lock);
			LOG_INFO("Client on socket: %lld is pinging server", SI->Socket);
			LeaveCriticalSection(&lock);
			break;
		}
		}
	}

	void Server::OnClientValidated(SOCKET_INFORMATION*& SI)
	{
		network::message<MessageType> msg = {};
		msg.header.id = MessageType::Client_Accepted;
		this->SendToClient(SI, msg);

		EnterCriticalSection(&lock);
		LOG_INFO("Client has been validated on socket %lld", SI->Socket);
		LeaveCriticalSection(&lock);
	}
}