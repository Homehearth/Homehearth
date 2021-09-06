#pragma once

class Server : public network::server_interface<network::MessageType>
{
private:

public:
	Server();
	virtual ~Server();
	// Inherited via server_interface
	virtual void OnClientConnect(const std::string& ip, const uint16_t& port) override;

	virtual void OnClientDisconnect() override;

	virtual void OnMessageReceived(const SOCKET& socketId, const network::message<network::MessageType>& msg) override;

};