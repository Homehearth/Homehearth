#pragma once

class Client :public network::client_interface<network::MessageType>
{
private:
	void Update();

public:
	Client();
	virtual ~Client();


	// Inherited via client_interface
	virtual void OnMessageReceived(const network::message<network::MessageType>& msg) override;
};