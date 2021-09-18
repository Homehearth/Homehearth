#pragma once

namespace network
{
	class Client :public client_interface<MessageType>
	{
	private:

	public:
		Client();
		virtual ~Client();

		virtual void OnValidation() override;
		// Inherited via client_interface
		virtual void OnMessageReceived(const message<MessageType>& msg) override;

		// Inherited via client_interface
		virtual void OnConnect() override;

		// Inherited via client_interface
		virtual void OnDisconnect() override;
	};

	Client::Client()
	{
	}

	void Client::OnDisconnect()
	{
		std::cout << "Disconnected from server!" << std::endl;
	}

	Client::~Client()
	{
	}

	void Client::OnValidation()
	{
		std::cout << "Connection has been validated!" << std::endl;
	}

	void Client::OnMessageReceived(const network::message<MessageType>& msg)
	{
		switch (msg.header.id)
		{
		case MessageType::Connected:
			std::cout << "Server sent: " << msg << std::endl;
			break;
		}
	}

	void Client::OnConnect()
	{
		std::cout << "Connected to server!" << std::endl;
	}
}