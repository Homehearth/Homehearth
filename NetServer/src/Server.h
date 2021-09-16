#pragma once
#include "NetServerPCH.h"
#include "Logger.h"

template <typename T>
class Server : public network::server_interface<T>
{
private:

public:
	Server();
	virtual ~Server();

	bool IsRunning();
	void Start(uint16_t port);
	void Stop();

	// Inherited via server_interface
	virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;
	virtual void OnClientDisconnect() override;
	virtual void OnMessageReceived(const SOCKET& socketId, CHAR* buffer, DWORD bytesReceived) override;
	virtual void OnClientValidated(const SOCKET& s) override;
};

template <typename T>
Server<T>::Server()
{
}

template <typename T>
bool Server<T>::IsRunning()
{
	return server_interface<T>::IsRunning();
}

template <typename T>
Server<T>::~Server()
{
}

template <typename T>
void Server<T>::OnClientConnect(std::string&& ip, const uint16_t& port)
{
	LOG_INFO("Client connected from %s:%d", ip.c_str(), port);
	//Broadcast();
}

template <typename T>
void Server<T>::OnClientDisconnect()
{
	LOG_INFO("Client disconnected!");
}

template <typename T>
void Server<T>::OnMessageReceived(const SOCKET& socketId, CHAR* buffer, DWORD bytesReceived)
{
	LOG_INFO("Message received: %s Bytes: %ld", buffer, bytesReceived);
	network::server_interface<T>::Send(socketId, buffer, bytesReceived);
}

template <typename T>
void Server<T>::OnClientValidated(const SOCKET& s)
{
	//using namespace network;
	//message<MessageType> msg = {};
	//msg.header.id = MessageType::Client_Accepted;
	//Send(s, msg);

	//std::cout << "Client has been validated!" << std::endl;
}

template <typename T>
void Server<T>::Start(uint16_t port)
{
	network::server_interface<T>::Start(port);
}

template <typename T>
void Server<T>::Stop()
{
	network::server_interface<T>::Stop();
}