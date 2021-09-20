#pragma once
#include "EnginePCH.h"

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
		LOG_INFO("Disconnected from the server!");
	}

	Client::~Client()
	{
	}

	void Client::OnValidation()
	{
		LOG_INFO("Your connection has been validated!");
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