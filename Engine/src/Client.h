#pragma once
#include "net_client_interface.h"
#include "EnginePCH.h"

using namespace network;

template <typename T>
class Client :public client_interface<T>
{
private:
	std::chrono::system_clock::time_point timeThen;

private:
	// Inherited via client_interface
	virtual void OnValidation() override;
	virtual void OnMessageReceived(message<T>& msg) override;
	virtual void OnConnect() override;
	virtual void OnDisconnect() override;

public:
	Client();
	virtual ~Client();
	Client& operator=(const Client& other) = delete;
	Client(const Client& other) = delete;

	void PingServer();
};

template <typename T>
Client<T>::Client()
{
}

template <typename T>
void Client<T>::OnDisconnect()
{
	LOG_INFO("Disconnected from the server!");
}

template <typename T>
Client<T>::~Client()
{
}

template <typename T>
void Client<T>::PingServer()
{
	message<MessageType> msg = {};
	msg.header.id = MessageType::Server_GetPing;
	this->timeThen = std::chrono::system_clock::now();
	this->Send(msg);
	EnterCriticalSection(&lock);
	LOG_INFO("PINGING");
	LeaveCriticalSection(&lock);
}

template <typename T>
void Client<T>::OnValidation()
{
	EnterCriticalSection(&lock);
	LOG_INFO("Your connection has been validated!");
	LeaveCriticalSection(&lock);
}

template <typename T>
void Client<T>::OnMessageReceived(message<T>& msg)
{
	switch (msg.header.id)
	{
	case MessageType::Client_Accepted:
	{
		EnterCriticalSection(&lock);
		LOG_INFO("You are validated!");
		LeaveCriticalSection(&lock);
		break;
	}
	case MessageType::Server_GetPing:
	{
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		EnterCriticalSection(&lock);
		LOG_INFO("Ping: %fs", std::chrono::duration<double>(timeNow - this->timeThen).count());
		LeaveCriticalSection(&lock);
		break;
	}
	}
}

template <typename T>
void Client<T>::OnConnect()
{
	EnterCriticalSection(&lock);
	LOG_INFO("Connected to the server!");
	LeaveCriticalSection(&lock);
}