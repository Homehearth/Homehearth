#include "NetServerPCH.h"
#include "Server.h"

Server::Server()
{
	m_uniqueID = 0;
}


Server::~Server()
{
}

void Server::Update(size_t nMaxMessage)
{
	while (IsRunning())
	{
		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessage && !m_qMessagesIn.empty())
		{
			auto msg = m_qMessagesIn.pop_front();

			this->OnMessageReceived(msg.remote, msg.msg);

			nMessageCount++;
		}
	}
}

void Server::OnClientConnect(std::string&& ip, const uint16_t& port)
{
	EnterCriticalSection(&lock);
	LOG_INFO("Client connected from %s:%d", ip.c_str(), port);
	LeaveCriticalSection(&lock);
}

void Server::OnClientDisconnect()
{
	LOG_INFO("Client disconnected!");
}

void Server::OnMessageReceived(const SOCKET& socket, message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Server_GetPing:
	{
		message<GameMsg> msg = {};
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

void Server::OnClientValidated(const SOCKET& socket)
{
	EnterCriticalSection(&lock);
	m_uniqueID++;
	LeaveCriticalSection(&lock);
	message<GameMsg> msg = {};
	msg << m_uniqueID;
	msg.header.id = GameMsg::Server_AssignID;
	this->SendToClient(socket, msg);

	LOG_INFO("Client has been validated on socket %lld", socket);
}