#pragma once

namespace network
{
	class Server : public server_interface<MessageType>
	{
	private:

	private:
		// Inherited via server_interface
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;
		virtual void OnClientDisconnect() override;
		virtual void OnMessageReceived(SOCKET_INFORMATION*& SI, message<MessageType>& msg) override;
		virtual void OnClientValidated(SOCKET_INFORMATION*& SI) override;

	public:
		Server();
		virtual ~Server();

		Server& operator=(const Server& other) = delete;
		Server(const Server& other) = delete;
	};

}