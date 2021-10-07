#include "EnginePCH.h"
#include "Client.h"

Client::Client()
{
}

void Client::OnDisconnect()
{
	LOG_INFO("Disconnected from the server!");
}

Client::~Client()
{
}

void Client::PingServer()
{
	message<GameMsg> msg = {};
	msg.header.id = GameMsg::Server_GetPing;
	this->timeThen = std::chrono::system_clock::now();
	Send(msg);
}

void Client::OnValidation()
{
	EnterCriticalSection(&lock);
	LOG_INFO("Your connection has been validated!");
	LeaveCriticalSection(&lock);
}

void Client::OnMessageReceived(message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Client_Accepted:
	{
		EnterCriticalSection(&lock);
		LOG_INFO("You are validated!");
		LeaveCriticalSection(&lock);
		break;
	}
	case GameMsg::Server_GetPing:
	{
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		EnterCriticalSection(&lock);
		LOG_INFO("Ping: %fs", std::chrono::duration<double>(timeNow - this->timeThen).count());
		LeaveCriticalSection(&lock);
		break;
	}
	}
}

void Client::OnConnect()
{
	EnterCriticalSection(&lock);
	LOG_INFO("Connected to the server!");
	LeaveCriticalSection(&lock);
}