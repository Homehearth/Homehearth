#pragma once
#include "NetServerPCH.h"

namespace network
{
	// TODO
	// SIGNAL THREADS IF THE SERVER HAS SHUTDOWN TO AVOID MEMORY LEAKS

	class Server : public server_interface<MessageType>
	{
	private:

	public:
		Server();
		virtual ~Server();

		Server& operator=(const Server& other) = delete;
		Server(const Server& other) = delete;

		// Inherited via server_interface
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;
		virtual void OnClientDisconnect() override;
		virtual void OnMessageReceived(const SOCKET& socketId, message<MessageType>& msg) override;
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
			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
			msg << timeNow;
			Send(socketId, msg);
			EnterCriticalSection(&lock);
			LOG_INFO("Client on socket: %lld is pinging server", socketId);
			LeaveCriticalSection(&lock);
			break;
		}
		}
	}

	void Server::OnClientValidated(const SOCKET& s)
	{
		network::message<MessageType> msg = {};
		msg.header.id = MessageType::Client_Accepted;
		Send(s, msg);

		EnterCriticalSection(&lock);
		LOG_INFO("Client has been validated on socket %lld", s);
		LeaveCriticalSection(&lock);
	}

}