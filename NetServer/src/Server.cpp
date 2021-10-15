#include "NetServerPCH.h"
#include "Server.h"

Server::Server(std::function<void(message<GameMsg>&)> handler)
	:server_interface<GameMsg>(handler)
{
	m_uniqueID = 0;
}

Server::~Server()
{
}

void Server::Update(size_t nMaxMessage)
{
	size_t nMessageCount = 0;
	while (nMessageCount < nMaxMessage && !m_qMessagesIn.empty())
	{
		auto msg = m_qMessagesIn.pop_front();

		this->OnMessageReceived(msg);

		nMessageCount++;
	}
}

SOCKET Server::GetConnection(uint32_t playerID) const
{
	if (connections.find(playerID) != connections.end())
	{
		return connections.at(playerID);
	}
	else
	{
		return INVALID_SOCKET;
	}
}

void Server::OnClientConnect(std::string&& ip, const uint16_t& port)
{
	EnterCriticalSection(&lock);
	LOG_INFO("Client connected from %s:%d", ip.c_str(), port);
	LeaveCriticalSection(&lock);
}

void Server::OnClientDisconnect(const SOCKET& socket)
{
	LOG_INFO("Client disconnected!");
}

void Server::OnMessageReceived(message<GameMsg>& msg)
{
	this->messageReceivedHandler(msg);
}

void Server::OnClientValidated(SOCKET_INFORMATION*& SI)
{
	EnterCriticalSection(&lock);
	m_uniqueID++;
	LeaveCriticalSection(&lock);
	message<GameMsg> msg = {};
	msg << m_uniqueID;
	msg.header.id = GameMsg::Server_AssignID;
	connections[m_uniqueID] = SI->Socket;
	SI->clientID = m_uniqueID;
	this->SendToClient(SI->Socket, msg);

	LOG_INFO("Client has been validated on socket %lld", SI->Socket);
}