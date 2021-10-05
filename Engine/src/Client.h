#pragma once
#include "net_client_interface.h"

using namespace network;

class Client :public network::client_interface<GameMsg>
{
private:
	std::chrono::system_clock::time_point timeThen;

private:
	// Inherited via client_interface
	virtual void OnValidation() override;
	virtual void OnMessageReceived(message<GameMsg > & msg) override;
	virtual void OnConnect() override;
	virtual void OnDisconnect() override;

public:
	Client();
	virtual ~Client();
	Client& operator=(const Client& other) = delete;
	Client(const Client& other) = delete;

	void PingServer();
};