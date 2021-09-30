#pragma once

namespace
{
	using namespace network;
	class Client :public client_interface<MessageType>
	{
	private:
		std::chrono::system_clock::time_point timeThen;

	private:
		// Inherited via client_interface
		virtual void OnValidation() override;
		virtual void OnMessageReceived(message<MessageType>& msg) override;
		virtual void OnConnect() override;
		virtual void OnDisconnect() override;

	public:
		Client();
		virtual ~Client();
		Client& operator=(const Client& other) = delete;
		Client(const Client& other) = delete;

		void PingServer();
		void TestServerWithGibberishData();
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

	inline void Client::PingServer()
	{
		message<MessageType> msg = {};
		msg.header.id = MessageType::Server_GetPing;
		this->timeThen = std::chrono::system_clock::now();
		this->Send(msg);
	}

	inline void Client::TestServerWithGibberishData()
	{
		message<MessageType> msg = {};
		msg.header.id = MessageType::Unknown;
		char text[] = "Rofl this is a nice time to be alive and the thing is with that is that Im a cool guy that walks a lot because its fun!!!!! :)";
		msg << text;
		this->Send(msg);
	}

	void Client::OnValidation()
	{
		EnterCriticalSection(&lock);
		LOG_INFO("Your connection has been validated!");
		LeaveCriticalSection(&lock);
	}

	void Client::OnMessageReceived(network::message<MessageType>& msg)
	{
		switch (msg.header.id)
		{
		case MessageType::Client_Accepted:
		{
			EnterCriticalSection(&lock);
			LOG_INFO("You are validated!");
			LeaveCriticalSection(&lock);
			break;
		}
		case MessageType::Server_GetPing:
		{
			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

			EnterCriticalSection(&lock);
			LOG_INFO("Ping: %fs", std::chrono::duration<double>(timeNow - this->timeThen).count());
			LeaveCriticalSection(&lock);
			break;
		}
		case MessageType::Unknown:
		{
			EnterCriticalSection(&lock);
			LOG_INFO("Gibberish success!");
			LeaveCriticalSection(&lock);
			break;
		}
		}
	}

	void Client::OnConnect()
	{
		EnterCriticalSection(&lock);
		LOG_INFO("Connected to the server!");
		LeaveCriticalSection(&lock);
	}
}