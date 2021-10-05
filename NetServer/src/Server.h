#pragma once
#include "net_server_interface.h"

using namespace network;

class Server : public server_interface<GameMsg>
{
private:
	uint32_t m_uniqueID;

private:
	// Inherited via server_interface
	virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;
	virtual void OnClientDisconnect() override;
	virtual void OnMessageReceived(const SOCKET& socket, message<GameMsg>& msg) override;
	virtual void OnClientValidated(const SOCKET& socket) override;

public:
	Server();
	virtual ~Server();

	Server& operator=(const Server& other) = delete;
	Server(const Server& other) = delete;

	// Defaulted to -1 which is translated to a huge nr (recall size_t is unsigned)
	void Update(size_t nMaxMessage = -1);
};