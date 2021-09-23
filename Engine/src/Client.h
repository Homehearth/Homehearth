#pragma once

using namespace network;

namespace
{
	class Client :public client_interface<MessageType>
	{
	private:

	public:
		Client();
		virtual ~Client();

		std::chrono::system_clock::time_point timeThen;
		Client& operator=(const Client& other) = delete;
		Client(const Client& other) = delete;

		// Inherited via client_interface
		virtual void OnValidation() override;

		// Inherited via client_interface
		virtual void OnMessageReceived(message<MessageType>& msg) override;

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

	void Client::OnMessageReceived(network::message<MessageType>& msg)
	{
		switch (msg.header.id)
		{
		case MessageType::Client_Accepted:
		{
			LOG_INFO("You are validated!");
			break;
		}
		case MessageType::PingServer:
		{
			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

			LOG_INFO("Ping: %fs", std::chrono::duration<double>(timeNow - this->timeThen).count());

			break;
		}
		}
	}

	void Client::OnConnect()
	{
		LOG_INFO("Connected to the server!");
	}
}