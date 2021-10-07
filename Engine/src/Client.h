#pragma once
#include "net_client_interface.h"

using namespace network;

class Client :public network::client_interface<GameMsg>
{
private:

private:
	// Inherited via client_interface
	virtual void OnValidation() override;
	virtual void OnMessageReceived(message<GameMsg>& msg) override;
	virtual void OnConnect() override;
	virtual void OnDisconnect() override;

public:
	Client() = default;
	Client(std::function<void(message<GameMsg>&)> handler);
	virtual ~Client();
	Client& operator=(const Client& other) = delete;
	Client(const Client& other) = delete;

	void Update(size_t nMaxMessage = -1);
};