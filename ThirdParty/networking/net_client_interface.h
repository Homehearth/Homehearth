#pragma once
#include "net_common.h"

namespace network
{
	template <typename T>
	class client_interface
	{
	private:
		SOCKET m_socket;
		std::function<void(const network::message<T>&)> MessageReceivedHandler;
		std::function<void()> OnConnectHandler;
		std::function<void()> OnDisconnectHandler;
		fd_set m_master;
		struct sockaddr_in m_endpoint;
		socklen_t m_endpointLen;

		// REMOVE LATER
		bool key[3] = { false, false, false };
		bool old_key[3] = { false, false, false };

	private:
		std::string PrintSocketData(struct addrinfo* p);
		void InitWinsock();
		SOCKET CreateSocket(std::string& ip, uint16_t& port);

	public:
		client_interface()
		{
			m_socket = INVALID_SOCKET;
			using namespace std::placeholders;
			MessageReceivedHandler = std::bind(&client_interface::OnMessageReceived, this, _1);
			OnConnectHandler = std::bind(&client_interface::OnConnect, this);
			OnDisconnectHandler = std::bind(&client_interface::OnDisconnect, this);

			ZeroMemory(&m_master, sizeof(m_master));
			m_endpointLen = sizeof(m_endpoint);
			ZeroMemory(&m_endpoint, m_endpointLen);

			InitWinsock();
		}

		virtual ~client_interface()
		{
			Disconnect();
			WSACleanup();
		}

	public:
		virtual void OnMessageReceived(const network::message<T>& msg) = 0;
		
		virtual void OnConnect() = 0;

		virtual void OnDisconnect() = 0;

		// Given IP and port establish a connection to the server
		bool Connect(std::string&& ip, uint16_t&& port);

		// Disconnect from the server
		void Disconnect();

		// Check to see if client is connected to a server
		bool IsConnected();

		void Send(const message<T>& msg);

		void UpdateClient(const bool& condition);
	};

	template <typename T>
	SOCKET client_interface<T>::CreateSocket(std::string& ip, uint16_t& port)
	{
		SOCKET serverSocket = INVALID_SOCKET;
		// Get a linked network structure based on provided hints
		struct addrinfo hints, * servinfo, * p;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

#ifdef _DEBUG
		std::cout << "Creating a client.." << std::endl;
#endif

		int8_t rv = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			std::cerr << "Addrinfo: " << WSAGetLastError() << std::endl;
			return false;
		}

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
#ifdef _DEBUG
			std::cout << PrintSocketData(p) << std::endl;
#endif
			serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (serverSocket == INVALID_SOCKET)
			{
				continue;
			}
			break;
		}

		if (p == nullptr)
		{
			return INVALID_SOCKET;
		}

		m_endpoint = *(struct sockaddr_in*)p->ai_addr;
		m_endpointLen = sizeof(m_endpoint);

		u_long enable = 1;
		ioctlsocket(m_socket, FIONBIO, &enable);

		return serverSocket;
	}

	template <typename T>
	void client_interface<T>::UpdateClient(const bool& condition)
	{
		message<T> msg;
		while (condition == true)
		{
			ZeroMemory(&msg, sizeof(msg));
			fd_set copy = m_master;

			int8_t socketCount = select(0, &copy, nullptr, nullptr, nullptr);

			if (socketCount == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					continue;
				}
				Disconnect();
				break;
			}

			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;

			if (key[0] && !old_key[0])
			{
				message<T> msg = {};
				msg.header.id = MessageType::PingServer;

				Send(msg);
			}

			for (int i = 0; i < 3; i++)
			{
				old_key[i] = key[i];
			}

			for (int i = 0; i < socketCount; i++)
			{
				SOCKET currentSocket = copy.fd_array[i];

				if (FD_ISSET(currentSocket, &copy))
				{
					// If the currentsocket is the socket client is bound to we have an incoming message from the server
					if (currentSocket == m_socket)
					{
						int32_t bytesLeft = recv(m_socket, (char*)&msg, sizeof(msg.header), 0);

						if (bytesLeft > 0)
						{
							if (this->MessageReceivedHandler != nullptr)
							{
								this->MessageReceivedHandler(msg);
							}
						}
						else
						{
							if (WSAGetLastError() == EWOULDBLOCK)
							{
								continue;
							}

							Disconnect();
						}
					}
				}
			}
		}
	}

	template<typename T>
	void client_interface<T>::InitWinsock()
	{
		// Initialize winsock
		WSADATA wsaData;

		WORD version = MAKEWORD(2, 2);

		int8_t rv = WSAStartup(version, &wsaData);

		if (rv != 0)
		{
#ifdef _DEBUG
			std::cout << "WSAStartup error code: " << WSAGetLastError() << std::endl;
#endif
		}

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
#ifdef _DEBUG
			std::cout << "Could not find a usable version of Winsock.dll" << std::endl;
#endif
		}
	}

	template<typename T>
	inline std::string client_interface<T>::PrintSocketData(addrinfo* p)
	{
		std::string data = "Full socket information:\n";

		if (p->ai_family == AF_INET)
		{
			data += "Address family: AF_INET\n";
		}
		else if (p->ai_family == AF_INET6)
		{
			data += "Address family: AF_INET6\n";
		}

		if (p->ai_socktype == SOCK_STREAM)
		{
			data += "Socktype: TCP";
		}
		else if (p->ai_socktype == SOCK_DGRAM)
		{
			data += "Socktype: UDP";
		}

		data += "\n";

		return data;
	}

	template<typename T>
	inline void client_interface<T>::Send(const message<T>& msg)
	{
		send(m_socket, (char*)&msg, sizeof(msg.header), 0);
	}

	template<typename T>
	inline bool client_interface<T>::Connect(std::string&& ip, uint16_t&& port)
	{
		m_socket = CreateSocket(ip, port);

		if (connect(m_socket, (struct sockaddr*)&m_endpoint, m_endpointLen) != 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
#ifdef _DEBUG
				std::cout << "Connect error code: " << WSAGetLastError() << std::endl;
#endif			
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;

				return false;
			}
		}

		FD_SET(m_socket, &m_master);

		// Callback to signal that client successfully connected to the server
		this->OnConnectHandler();

		return true;
	}

	template<typename T>
	inline void client_interface<T>::Disconnect()
	{
		if (!IsConnected())
		{
			return;
		}

		if (closesocket(m_socket) != 0)
		{
#ifdef _DEBUG
			std::cout << "Failed to close socket: " << WSAGetLastError() << std::endl;
#endif
		}
		this->OnDisconnectHandler();
		m_socket = INVALID_SOCKET;
	}

	template<typename T>
	inline bool client_interface<T>::IsConnected()
	{
		if (m_socket == INVALID_SOCKET)
		{
			return false;
		}

		return true;
	}
}