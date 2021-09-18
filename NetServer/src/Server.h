#pragma once
#include "NetServerPCH.h"

namespace network
{
	class Server : public server_interface<MessageType>
	{
	private:

	public:
		Server();
		virtual ~Server();

		// Inherited via server_interface
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;
		virtual void OnClientDisconnect() override;
		virtual void OnMessageReceived(const SOCKET& socketId, CHAR* buffer, DWORD bytesReceived) override;
		virtual void OnClientValidated(const SOCKET& s) override;
	};

	Server::Server()
	{
	}

	Server::~Server()
	{
	}

	void Server::OnClientConnect(std::string&& ip, const uint16_t& port)
	{
		LOG_INFO("Client connected from %s:%d", ip.c_str(), port);
		//Broadcast();
	}

	void Server::OnClientDisconnect()
	{
		LOG_INFO("Client disconnected!");
	}

	void Server::OnMessageReceived(const SOCKET& socketId, CHAR* buffer, DWORD bytesReceived)
	{
		LOG_INFO("Message received: %s Bytes: %ld", buffer, bytesReceived);
		/*Send(socketId, buffer, bytesReceived);*/
	}

	void Server::OnClientValidated(const SOCKET& s)
	{
		network::message<MessageType> msg = {};
		msg.header.id = MessageType::Client_Accepted;
		Send(s, msg);

		LOG_INFO("Client has been validated!");
	}

}