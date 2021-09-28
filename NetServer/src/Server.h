#pragma once
#include "NetServerPCH.h"

namespace network
{
	class Server : public server_interface<MessageType>
	{
	private:

	private:
		// Inherited via server_interface
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;
		virtual void OnClientDisconnect() override;
		virtual void OnMessageReceived(const SOCKET& socketId, message<MessageType>& msg) override;
		virtual void OnClientValidated(const SOCKET& s) override;

	public:
		Server();
		virtual ~Server();

		Server& operator=(const Server& other) = delete;
		Server(const Server& other) = delete;

	};

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
		//Broadcast();
	}

	void Server::OnClientDisconnect()
	{
		EnterCriticalSection(&lock);
		LOG_INFO("Client disconnected!");
		LeaveCriticalSection(&lock);
	}

	void Server::OnMessageReceived(const SOCKET& socketId, message<MessageType>& msg)
	{
		switch (msg.header.id)
		{
		case MessageType::PingServer:
		{
			message<MessageType> msg = {};
			msg.header.id = MessageType::PingServer;
			this->SendToClient(socketId, msg);
			EnterCriticalSection(&lock);
			LOG_INFO("Client on socket: %lld is pinging server", socketId);
			LeaveCriticalSection(&lock);
			break;
		}
		case MessageType::Unknown:
		{		
			EnterCriticalSection(&lock);
			for (int i = 0; i < (int)msg.payload.size(); i++)
			{
				std::cout << msg.payload[i];
			}
			std::cout << std::endl;
			LeaveCriticalSection(&lock);
			this->SendToClient(socketId, msg);
			break;
		}
		}
	}

	void Server::OnClientValidated(const SOCKET& s)
	{
		network::message<MessageType> msg = {};
		msg.header.id = MessageType::Client_Accepted;
		this->SendToClient(s, msg);

		EnterCriticalSection(&lock);
		LOG_INFO("Client has been validated on socket %lld", s);
		LeaveCriticalSection(&lock);
	}

}