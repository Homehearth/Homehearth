#pragma once
#include "net_common.h"

namespace network
{
	template <typename T>
	class client_interface
	{
	private:
		SOCKET m_socket;
		std::function<void(const network::message<network::MessageType>&)> MessageReceivedHandler;
		fd_set m_master;

		// REMOVE LATER
		bool key[3] = { false, false, false };
		bool old_key[3] = { false, false, false };

	private:
		std::string PrintSocketData(struct addrinfo* p);
		void InitWinsock();

	public:
		client_interface()
		{
			m_socket = INVALID_SOCKET;

			InitWinsock();

			using namespace std::placeholders;
			MessageReceivedHandler = std::bind(&client_interface::OnMessageReceived, this, _1);

			ZeroMemory(&m_master, sizeof(m_master));
		}

		virtual ~client_interface()
		{
			Disconnect();
			WSACleanup();
		}

	public:
		virtual void OnMessageReceived(const network::message<network::MessageType>& msg) = 0;

		// Given IP and port establish a connection to the server
		bool Connect(const std::string& ip, const uint16_t port);

		// Disconnect from the server
		void Disconnect();

		// Check to see if client is connected to a server
		bool IsConnected();

		void Send(const message<T>& msg);

		void UpdateClient(const bool& condition);
	};

	template <typename T>
	void client_interface<T>::UpdateClient(const bool& condition)
	{
		message<T> msg;
		while (condition == true)
		{
			ZeroMemory(&msg, sizeof(msg));
			fd_set copy = m_master;

			int8_t socketCount = select(0, &copy, nullptr, nullptr, nullptr);

			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;

			if (key[0] && !old_key[0])
			{
				message<MessageType> msg = {};
				msg.header.id = MessageType::PingServer;

				Send(msg);
			}

			for (int i = 0; i < 3; i++)
			{
				old_key[i] = key[i];
			}

			if (socketCount == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					continue;
				}
				std::cout << "Select: " << WSAGetLastError() << std::endl;
			}

			for (int i = 0; i < socketCount; i++)
			{
				SOCKET currentSocket = copy.fd_array[i];

				if (FD_ISSET(currentSocket, &copy))
				{
					// If the currentsocket is the socket client is bound to we have an incoming message from the server
					if (currentSocket == m_socket)
					{
						uint32_t bytesLeft = recv(m_socket, (char*)&msg, sizeof(msg.header), 0);

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
	inline bool client_interface<T>::Connect(const std::string& ip, const uint16_t port)
	{
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
			m_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (m_socket == INVALID_SOCKET)
			{
				continue;
			}
			u_long enable = 1;
			ioctlsocket(m_socket, FIONBIO, &enable);

			if (connect(m_socket, p->ai_addr, static_cast<int>(p->ai_addrlen)) != 0)
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

			break;
		}

		// Reached end of list and could not connect to any
		if (p == nullptr)
		{
#ifdef _DEBUG
			std::cout << "Failed to connect!" << std::endl;
#endif
			m_socket = INVALID_SOCKET;

			return false;
		}

		FD_SET(m_socket, &m_master);

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