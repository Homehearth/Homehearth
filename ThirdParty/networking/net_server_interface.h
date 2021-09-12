#pragma once
#include "net_common.h"

namespace network
{
	struct SOCKET_INFORMATION
	{
		CHAR Buffer[BUFFER_SIZE];
		WSABUF DataBuf;
		SOCKET Socket;
		DWORD BytesSEND;
		DWORD BytesRECV;
	};

	template <typename T>
	class server_interface
	{
	private:
		SOCKET m_listening;
		DWORD m_eventTotal;
		CRITICAL_SECTION CriticalSection;

		// Array storing all the events
		WSAEVENT m_Events[WSA_MAXIMUM_WAIT_EVENTS];

		// Container for all sockets that has an established connection
		SOCKET_INFORMATION* m_Sockets[WSA_MAXIMUM_WAIT_EVENTS] = { nullptr };

	private:
		SOCKET CreateSocket(const uint16_t& port);
		std::string PrintSocketData(struct addrinfo* p);
		void Listen(const uint32_t& nListen);
		void InitWinsock();
		bool CreateSocketInformation(SOCKET s);
		void ProcessIO();

	public:
		server_interface()
		{
			m_listening = INVALID_SOCKET;
			using namespace std::placeholders;
			MessageReceivedHandler = std::bind(&server_interface::OnMessageReceived, this, _1, _2);
			ClientConnectHandler = std::bind(&server_interface::OnClientConnect, this, _1, _2);
			ClientDisconnectHandler = std::bind(&server_interface::OnClientDisconnect, this);

			m_eventTotal = 0;
		}
		virtual ~server_interface()
		{
			WSACleanup();
		}

	public:
		void Start(const uint16_t& port);
		void Stop();
		void Broadcast();
		SOCKET WaitForConnection();
		void Send();

		// Pure virtuals that happen when an event occurs

		// Called once when a client connects
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) = 0;
		// Called once when a client connects
		virtual void OnClientDisconnect() = 0;
		// Called once when a message is received
		virtual void OnMessageReceived(const SOCKET& socketId, const message<T>& msg) = 0;

		std::function<void(const SOCKET&, const message<T>&)> MessageReceivedHandler;
		std::function<void(std::string&&, const uint16_t&)> ClientConnectHandler;
		std::function<void()> ClientDisconnectHandler;
	};

	template <typename T>
	bool server_interface<T>::CreateSocketInformation(SOCKET s)
	{
		// Fill in the details of our accepted socket
		SOCKET_INFORMATION* SI = new SOCKET_INFORMATION;

		SI->Socket = s;
		SI->BytesSEND = 0;
		SI->BytesRECV = 0;
		SI->DataBuf.len = BUFFER_SIZE;
		SI->DataBuf.buf = SI->Buffer;

		m_Events[m_eventTotal] = WSACreateEvent();

		WSAEventSelect(s, m_Events[m_eventTotal], FD_READ | FD_CLOSE);

		// Signal the first event in the event array to tell the worker thread to
		// service an additional event in the event array
		if (WSASetEvent(m_Events[0]) == FALSE)
		{
			printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
			return false;
		}
		else
		{
			printf("Don't worry, WSASetEvent() is OK!\n");
		}

		m_Sockets[m_eventTotal] = SI;

		m_eventTotal++;

		return true;
	}

	template <typename T>
	void server_interface<T>::Send()
	{

	}

	template <typename T>
	void server_interface<T>::Broadcast()
	{
		message<T> msg = { };
		msg.header.id = MessageType::Unknown;
	}

	template <typename T>
	void server_interface<T>::InitWinsock()
	{
		// Initialize winsock
		WSADATA wsaData;

		WORD version = MAKEWORD(2, 2);

		int8_t rv = WSAStartup(version, &wsaData);

		if (rv != 0)
		{
			std::cout << "WSAStartup error code: " << WSAGetLastError() << std::endl;

			return;
		}

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			std::cerr << "Could not find a usable version of Winsock.dll" << std::endl;

			return;
		}
	}

	template <typename T>
	void server_interface<T>::Listen(const uint32_t& nListen)
	{
		if (listen(m_listening, nListen) != 0)
		{
			std::cout << "Listen(): failed with error " << WSAGetLastError() << std::endl;
		}
	}

	template <typename T>
	SOCKET server_interface<T>::WaitForConnection()
	{
		SOCKET clientSocket = accept(m_listening, nullptr, nullptr);
		return clientSocket;
	}

	template <typename T>
	std::string server_interface<T>::PrintSocketData(addrinfo* p)
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
			data += "Socktype: SOCK_STREAM\n";
		}
		else if (p->ai_socktype == SOCK_DGRAM)
		{
			data += "Socktype: SOCK_DGRAM\n";
		}

		if (p->ai_protocol == IPPROTO_TCP)
		{
			data += "Protocol: TCP\n";
		}
		else if (p->ai_protocol == IPPROTO_UDP)
		{
			data += "Protocol: UDP\n";
		}

		char ipAsString[IPV6_ADDRSTRLEN] = {};

		inet_ntop(p->ai_family, get_in_addr(p->ai_addr), ipAsString, sizeof(ipAsString));

		data += "Host: ";
		data += ipAsString;
		data += ":";
		data += std::to_string(GetPort(p->ai_addr));

		data += "\n";

		return data;
	}

	template <typename T>
	SOCKET server_interface<T>::CreateSocket(const uint16_t& port)
	{
		// Get a linked network structure based on provided hints
		struct addrinfo hints, * servinfo, * p;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = IPPROTO_TCP;

		int8_t rv = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			std::cerr << "Addrinfo: " << gai_strerror(rv) << std::endl;
			return INVALID_SOCKET;
		}

		SOCKET listener;

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			std::cout << PrintSocketData(p) << std::endl;

			listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (listener == INVALID_SOCKET)
			{
				continue;
			}
			if (bind(listener, p->ai_addr, static_cast<int>(p->ai_addrlen)) != 0)
			{
				std::cout << "Bind error code: " << WSAGetLastError() << std::endl;
				closesocket(listener);
				continue;
			}

			break;
		}

		// Reached end of list and could not connect to any
		if (p == nullptr)
		{
			return INVALID_SOCKET;
		}

		freeaddrinfo(servinfo);

		return listener;
	}

	template <typename T>
	void server_interface<T>::ProcessIO()
	{
		DWORD index;
		SOCKET_INFORMATION* SI;
		WSANETWORKEVENTS wsaConnectEvents;
		WSANETWORKEVENTS wsaProcessEvents;

		while (1)
		{
			// Will put thread to sleep unless there is I/O to process or a new connection has been made
			if ((index = WSAWaitForMultipleEvents(m_eventTotal, m_Events, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
			{
				printf("WSAWaitForMultipleEvents() failed %d\n", WSAGetLastError());
				continue;
			}

			index = index - WSA_WAIT_EVENT_0;
			std::cout << "Event index: " << index << std::endl;

			// If the event triggered was zero then a connection attempt was made
			// on our listening socket. Because its the first event in the list
			if (index == 0)
			{
				WSAEnumNetworkEvents(m_listening, m_Events[index], &wsaConnectEvents);

				if (wsaConnectEvents.lNetworkEvents & FD_ACCEPT && (wsaConnectEvents.iErrorCode[FD_ACCEPT_BIT] == 0))
				{
					SOCKET clientSocket = WaitForConnection();

					struct sockaddr_in c = {};
					socklen_t cLen = sizeof(c);
					getpeername(clientSocket, (struct sockaddr*)&c, &cLen);
					char ipAsString[IPV6_ADDRSTRLEN] = {};
					inet_ntop(c.sin_family, &c.sin_addr, ipAsString, sizeof(ipAsString));
					uint16_t port = GetPort((struct sockaddr*)&c);

					this->ClientConnectHandler(ipAsString, port);

					EnterCriticalSection(&CriticalSection);

					if (!CreateSocketInformation(clientSocket))
					{
						continue;
					}

					LeaveCriticalSection(&CriticalSection);
				}

				continue;
			}

			SI = m_Sockets[index];

			WSAEnumNetworkEvents(SI->Socket, m_Events[index], &wsaProcessEvents);

			if (wsaProcessEvents.lNetworkEvents & FD_CLOSE && (wsaProcessEvents.iErrorCode[FD_CLOSE_BIT] == 0))
			{
				this->ClientDisconnectHandler();

				// De-allocate the structure for the connected client
				closesocket(SI->Socket);
				delete m_Sockets[index];
				m_Sockets[index] = nullptr;

				WSACloseEvent(m_Events[index]);

				EnterCriticalSection(&CriticalSection);

				// Cleanup SocketArray and EventArray by removing the socket event handle
				// and socket information structure if they are not at the end of the arrays
				if (index + 1 != m_eventTotal)
				{
					for (DWORD i = index; i < m_eventTotal; i++)
					{
						m_Events[i] = m_Events[i + 1];
						m_Sockets[i] = m_Sockets[i + 1];
					}
				}

				m_eventTotal--;

				LeaveCriticalSection(&CriticalSection);

				continue;
			}
			if (wsaProcessEvents.lNetworkEvents & FD_READ && (wsaProcessEvents.iErrorCode[FD_READ_BIT] == 0))
			{
				message<T> msg = {};

				int32_t bytes = recv(SI->Socket, (char*)&msg, sizeof(msg.header), 0);

				if (bytes > 0)
				{
					this->MessageReceivedHandler(SI->Socket, msg);
				}
				else
				{
					std::cout << "recv: " << WSAGetLastError() << std::endl;
				}
			}
		}
	}

	template <typename T>
	void server_interface<T>::Start(const uint16_t& port)
	{
		std::cout << "STARTING SERVER.." << std::endl;
		InitializeCriticalSection(&CriticalSection);
		InitWinsock();

		WSABUF buffer = {};
		m_listening = CreateSocket(port);

		// Options to disable Nagle's algorithm (can queue up multiple packets instead of sending 1 by 1)
		// SO_REUSEADDR will let the server to reuse the port its bound on even if it have not closed 
		// by the the operating system yet.
		const char enable = 1;
		setsockopt(m_listening, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
		setsockopt(m_listening, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

		this->Listen(SOMAXCONN);

		// Create an event for the accepting socket
		HANDLE connectionEvent = WSACreateEvent();

		if (connectionEvent == WSA_INVALID_EVENT)
		{
			std::cout << "Failed to create event " << WSAGetLastError() << std::endl;
			return;
		}

		WSAEventSelect(m_listening, connectionEvent, FD_ACCEPT);

		m_Events[0] = connectionEvent;

		DWORD ThreadId = 0;
		m_eventTotal++;

		ProcessIO();
	}

	template <typename T>
	void server_interface<T>::Stop()
	{

	}
}