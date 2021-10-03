#pragma once
#include "NetServerPCH.h"

using namespace network;

template <typename T>
class Server : public server_interface<T>
{
private:
	uint64_t m_uniqueID;

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
	m_uniqueID = 0;
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
	LOG_INFO("Client disconnected!");
}

template <typename T>
void Server<T>::OnMessageReceived(const SOCKET& socket, message<T>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Server_GetPing:
	{
		message<T> msg = {};
		msg.header.id = GameMsg::Server_GetPing;
		this->SendToClient(socket, msg);
		LOG_INFO("Client on socket: %lld is pinging server", socket);
		break;
	}
	case GameMsg::Game_MovePlayer:
	{
		LOG_INFO("Moving player!");
		break;
	}
	}
}

template <typename T>
void Server<T>::OnClientValidated(const SOCKET& socket)
{
	EnterCriticalSection(&lock);
	m_uniqueID++;
	LeaveCriticalSection(&lock);
	message<T> msg = {};
	msg << m_uniqueID;
	msg.header.id = GameMsg::Server_AssignID;
	this->SendToClient(socket, msg);

	LOG_INFO("Client has been validated on socket %lld", socket);
}