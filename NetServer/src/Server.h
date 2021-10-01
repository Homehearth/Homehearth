#pragma once
#include "NetServerPCH.h"

using namespace network;

template <typename T>
class Server : public server_interface<T>
{
private:
	uint64_t m_CurrentID;

private:
	// Inherited via server_interface
	virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;
	virtual void OnClientDisconnect() override;
	virtual void OnMessageReceived(const SOCKET& socket, message<T>& msg) override;
	virtual void OnClientValidated(const SOCKET& socket) override;

public:
	Server();
	virtual ~Server();

	Server& operator=(const Server& other) = delete;
	Server(const Server& other) = delete;
};

template <typename T>
Server<T>::Server()
{
	m_CurrentID = 0;
}

template <typename T>
Server<T>::~Server()
{
}

template <typename T>
void Server<T>::OnClientConnect(std::string&& ip, const uint16_t& port)
{
	EnterCriticalSection(&lock);
	LOG_INFO("Client connected from %s:%d", ip.c_str(), port);
	LeaveCriticalSection(&lock);
}

template <typename T>
void Server<T>::OnClientDisconnect()
{
	EnterCriticalSection(&lock);
	LOG_INFO("Client disconnected!");
	LeaveCriticalSection(&lock);
}

template <typename T>
void Server<T>::OnMessageReceived(const SOCKET& socket, message<T>& msg)
{
	switch (msg.header.id)
	{
	case MessageType::Server_GetPing:
	{
		message<T> msg = {};
		msg.header.id = MessageType::Server_GetPing;
		this->SendToClient(socket, msg);
		EnterCriticalSection(&lock);
		LOG_INFO("Client on socket: %lld is pinging server", socket);
		LeaveCriticalSection(&lock);
		break;
	}
	case MessageType::Game_MovePlayer:
	{
		EnterCriticalSection(&lock);
		LOG_INFO("Moving player!");
		LeaveCriticalSection(&lock);
		break;
	}
	}
}

template <typename T>
void Server<T>::OnClientValidated(const SOCKET& socket)
{
	EnterCriticalSection(&lock);
	uint64_t tempID = m_CurrentID++;
	LeaveCriticalSection(&lock);
	network::message<T> msg = {};
	msg.header.id = MessageType::Client_Accepted;
	this->SendToClient(socket, msg);

	EnterCriticalSection(&lock);
	LOG_INFO("Client has been validated on socket %lld", socket);
	LeaveCriticalSection(&lock);
}