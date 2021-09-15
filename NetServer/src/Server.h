#pragma once

class Server : public network::server_interface<network::MessageType>
{
private:
	Server();

	static Server* m_instance;

public:
	virtual ~Server();

	static Server* GetInstance();

	static bool IsRunning();

	static SOCKET WaitForConnection();

	// Inherited via server_interface
	virtual void OnClientConnect(std::string&& ip, const uint16_t& port) override;

	virtual void OnClientDisconnect() override;

	virtual void OnMessageReceived(const SOCKET& socketId, CHAR* buffer, DWORD bytesReceived) override;

	virtual void OnClientValidated(const SOCKET& s) override;

	static void Destroy();

	static void Start(uint16_t port);
	
	static void Stop();
};