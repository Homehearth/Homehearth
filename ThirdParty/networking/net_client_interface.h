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
		struct sockaddr_in m_endpoint;
		socklen_t m_endpointLen;
		uint64_t m_handshakeIn;
		uint64_t m_handshakeOut;
		CRITICAL_SECTION lock;

		WSAEVENT m_event;

		// REMOVE LATER
		bool key[3] = { false, false, false };
		bool old_key[3] = { false, false, false };

	private:
		std::string PrintSocketData(struct addrinfo* p);
		void InitWinsock();
		SOCKET CreateSocket(std::string& ip, uint16_t& port);
		void ProcessIO();

	public:
		client_interface()
		{
			m_socket = INVALID_SOCKET;
			using namespace std::placeholders;
			MessageReceivedHandler = std::bind(&client_interface::OnMessageReceived, this, _1);
			OnConnectHandler = std::bind(&client_interface::OnConnect, this);
			OnDisconnectHandler = std::bind(&client_interface::OnDisconnect, this);

			m_endpointLen = sizeof(m_endpoint);
			ZeroMemory(&m_endpoint, m_endpointLen);
			m_handshakeIn = 0;
			m_handshakeOut = 0;

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

		//virtual void OnValidation() = 0;

		// Given IP and port establish a connection to the server
		bool Connect(std::string&& ip, uint16_t&& port);

		// Disconnect from the server
		void Disconnect();

		// Check to see if client is connected to a server
		bool IsConnected();

		void Send(const message<T>& msg);
	};

	template <typename T>
	SOCKET client_interface<T>::CreateSocket(std::string& ip, uint16_t& port)
	{
		SOCKET sock = INVALID_SOCKET;
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
			sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (sock == INVALID_SOCKET)
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

		const char enable = 1;

		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
		setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

		return sock;
	}

	template <typename T>
	void client_interface<T>::ProcessIO()
	{
		DWORD index = 0;
		WSANETWORKEVENTS networkEvents = {};

		while (1)
		{
			// Will put thread to sleep unless there is I/O to process or a new connection has been made
			if ((index = WSAWaitForMultipleEvents(1, &m_event, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
			{
				printf("WSAWaitForMultipleEvents() failed %d\n", WSAGetLastError());
				continue;
			}

			index = index - WSA_WAIT_EVENT_0;

			WSAEnumNetworkEvents(m_socket, m_event, &networkEvents);

			if (networkEvents.lNetworkEvents & FD_CONNECT && (networkEvents.iErrorCode[FD_CONNECT_BIT] == 0))
			{
				this->OnConnectHandler();
			}

			if (networkEvents.lNetworkEvents & FD_READ && (networkEvents.iErrorCode[FD_READ_BIT] == 0))
			{
				if (!m_handshakeIn)
				{
					int16_t bytes = recv(m_socket, (char*)&m_handshakeIn, sizeof(uint64_t), 0);
					if (bytes > 0)
					{
						m_handshakeOut = scrambleData(m_handshakeIn);
						send(m_socket, (char*)&m_handshakeOut, sizeof(uint64_t), 0);

						message<MessageType> msg = {};
						recv(m_socket, (char*)&msg.header, sizeof(msg.header), 0);
						switch (msg.header.id)
						{
						case MessageType::Client_Accepted:
						{
							std::cout << "VALIDATED!" << std::endl;
						}
						}
					}
					else
					{
						Disconnect();
					}
				}
				//message<T> msg = {};
				//int32_t bytes = recv(m_socket, (char*)&msg.header, sizeof(msg.header), 0);
				//char buffer[4096] = {};
				//bytes = recv(m_socket, (char*)buffer, sizeof(msg.header.size - sizeof(msg.header)), 0);
				//msg.payload.resize(bytes);
				//memcpy(msg.payload.data(), buffer, bytes);

				//for (int i = 0; i < msg.payload.size(); i++)
				//{
				//	std::cout << msg.payload[i];
				//}
				//std::cout << std::endl;

				//this->MessageReceivedHandler(msg);
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

		InitializeCriticalSection(&lock);
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
		send(m_socket, (char*)&msg.header, static_cast<int>(sizeof(msg.header)), 0);
		send(m_socket, (char*)msg.payload.data(), static_cast<int>(sizeof(msg.payload.size())));
	}

	template<typename T>
	inline bool client_interface<T>::Connect(std::string&& ip, uint16_t&& port)
	{
		EnterCriticalSection(&lock);
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

		m_event = WSACreateEvent();

		WSAEventSelect(m_socket, m_event, FD_CONNECT | FD_READ | FD_CLOSE);

		if (WSASetEvent(m_event) == FALSE)
		{
			printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
			return false;
		}

		thread::MultiThreader::InsertJob(std::bind([this] { ProcessIO(); }));
		LeaveCriticalSection(&lock);

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