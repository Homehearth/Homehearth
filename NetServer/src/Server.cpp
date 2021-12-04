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
		msg >> msg.header.id;

		this->OnMessageReceived(msg);

		nMessageCount++;
	}
}

SOCKET Server::GetConnection(uint32_t playerID) const
{
	if (m_connections.find(playerID) != m_connections.end())
	{
		return m_connections.at(playerID);
	}
	else
	{
		return INVALID_SOCKET;
	}
}

uint32_t Server::PopNextUniqueID()
{
	uint32_t id = m_uniqueID;
	m_uniqueID++;
	return id;
}

void Server::OnClientConnect(std::string&& ip, const uint16_t& port)
{
	LOG_INFO("Client connected from %s:%d", ip.c_str(), port);
}

void Server::OnClientDisconnect(const SOCKET& socket)
{
	LOG_INFO("Client disconnected!");
}

void Server::OnMessageReceived(message<GameMsg>& msg)
{
	this->messageReceivedHandler(msg);
}

void Server::OnClientValidated(const SOCKET& socket)
{
	uint32_t id = this->PopNextUniqueID();
	message<GameMsg> msg = {};
	msg << id;
	msg.header.id = GameMsg::Server_AssignID;
	m_connections[id] = socket;
	this->SendToClient(id, msg);

	LOG_INFO("Client has been validated on socket %lld", socket);
}