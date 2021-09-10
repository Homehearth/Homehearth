#pragma once
#include "net_common.h"

namespace network
{
	typedef struct _SOCKET_INFORMATION
	{
		CHAR Buffer[BUFFER_SIZE];
		WSABUF DataBuf;
		SOCKET Socket;
		WSAOVERLAPPED Overlapped;
		DWORD BytesSEND;
		DWORD BytesRECV;

	} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

	template <typename T>
	class server_interface
	{
	private:

		DWORD EventTotal;
		// Essentially a mutex
		CRITICAL_SECTION CriticalSection;
		// Array storing all the events
		WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
		WSABUF d;

		// Container for all sockets that has an established connection
		LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
		// Function that will process all IO when event has been signaled
		DWORD WINAPI ProcessIO();

		// Struct containing all needed information regarding a client
		SOCKET m_listen;

	private:
		SOCKET CreateSocket(const uint16_t& port);
		std::string PrintSocketData(struct addrinfo* p);
		void Listen(const uint32_t& nListen);
		void InitWinsock();

	public:
		server_interface()
		{
			m_listen = INVALID_SOCKET;
			message<MessageType> msg;
			using namespace std::placeholders;
			MessageReceivedHandler = std::bind(&server_interface::OnMessageReceived, this, _1, _2);
			ClientConnectHandler = std::bind(&server_interface::OnClientConnect, this, _1, _2);
			ClientDisconnectHandler = std::bind(&server_interface::OnClientDisconnect, this);

			EventTotal = 0;
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

		// Pure virtuals that happen when an event occurs

		// Called once when a client connects
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) = 0;
		// Called once when a client connects
		virtual void OnClientDisconnect() = 0;
		// Called once when a message is received
		virtual void OnMessageReceived(const SOCKET& socketId, const message<MessageType>& msg) = 0;

		std::function<void(const SOCKET&, const message<MessageType>&)> MessageReceivedHandler;
		std::function<void(std::string&&, const uint16_t&)> ClientConnectHandler;
		std::function<void()> ClientDisconnectHandler;
	};

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
		if (listen(m_listen, nListen) != 0)
		{
			std::cout << "Listen(): failed with error " << WSAGetLastError() << std::endl;
		}
	}

	template <typename T>
	SOCKET server_interface<T>::WaitForConnection()
	{
		SOCKET clientSocket = accept(m_listen, nullptr, nullptr);
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
			data += "Socktype: TCP\n";
		}
		else if (p->ai_socktype == SOCK_DGRAM)
		{
			data += "Socktype: UDP\n";
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

			listener = WSASocket(p->ai_family, p->ai_socktype, p->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

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
	void server_interface<T>::Start(const uint16_t& port)
	{
		std::cout << "STARTING SERVER.." << std::endl;
		InitializeCriticalSection(&CriticalSection);
		InitWinsock();

		WSABUF buffer = {};
		m_listen = CreateSocket(port);

		// Options to disable Nagle's algorithm (can queue up multiple packets instead of sending 1 by 1)
		// SO_REUSEADDR will let the server to reuse the port its bound on even if it have not closed 
		// by the the operating system yet.
		u_long enable = 1;
		setsockopt(m_listen, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&enable), sizeof(enable));
		setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable));

		Listen(SOMAXCONN);

		// Create an event for the accepting socket
		if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
		{
			std::cout << "Failed to create event " << WSAGetLastError() << std::endl;
		}

		DWORD ThreadId = 0;
		EventTotal = 1;

		thread::MultiThreader::InsertJob(std::bind([this] { ProcessIO(); }));

		// This will run and accept incoming connections
		while (1)
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
			SocketArray[EventTotal] = new _SOCKET_INFORMATION;
			// Fill in the details of our accepted socket

			SocketArray[EventTotal]->Socket = clientSocket;
			ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
			SocketArray[EventTotal]->BytesSEND = 0;
			SocketArray[EventTotal]->BytesRECV = 0;
			SocketArray[EventTotal]->DataBuf.len = BUFFER_SIZE;
			SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;

			EventArray[EventTotal] = WSACreateEvent();
			SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal];

			// Post a WSARecv() request to to begin receiving data on the socket

			DWORD Flags = 0;
			DWORD RecvBytes = 0;

			std::cout << "Receiving from: " << SocketArray[EventTotal]->Socket << std::endl;

			if (WSARecv(SocketArray[EventTotal]->Socket,
				&(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags, &(SocketArray[EventTotal]->Overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					printf("WSARecv() failed with error %d\n", WSAGetLastError());

					return;
				}
				else
				{
					std::cout << "WSARecv Error pending..." << std::endl;
				}
			}
			else
			{
				printf("WSARecv() should be working!\n");
			}

			EventTotal++;

			LeaveCriticalSection(&CriticalSection);

			// Signal the first event in the event array to tell the worker thread to
			// service an additional event in the event array
			if (WSASetEvent(EventArray[0]) == FALSE)
			{
				printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
				return;
			}
			else
			{
				printf("Don't worry, WSASetEvent() is OK!\n");
			}
		}
	}

	template <typename T>
	void server_interface<T>::Stop()
	{

	}

	template <typename T>
	DWORD WINAPI server_interface<T>::ProcessIO()
	{
		DWORD index;
		DWORD Flags;
		LPSOCKET_INFORMATION SI;
		DWORD BytesTransferred;
		DWORD i;
		DWORD RecvBytes, SendBytes;

		// Process asynchronous WSASend, WSARecv requests
		while (1)
		{
			if ((index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
			{
				printf("WSAWaitForMultipleEvents() failed %d\n", WSAGetLastError());

				continue;
			}
			else
			{
				index = index - WSA_WAIT_EVENT_0;
				
				std::cout << "Event index: " << index << std::endl;
			}

			// If the event triggered was zero then a connection attempt was made
			// on our listening socket.
			SI = SocketArray[index];

			if (index == 0)
			{
				WSAResetEvent(EventArray[0]);

				continue;
			}

			WSAResetEvent(EventArray[index]);

			if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred, FALSE, &Flags) == FALSE || BytesTransferred == 0)
			{
				this->ClientDisconnectHandler();
				closesocket(SI->Socket);

				delete SI;

				WSACloseEvent(EventArray[index]);

				// Cleanup SocketArray and EventArray by removing the socket event handle
				// and socket information structure if they are not at the end of the arrays

				EnterCriticalSection(&CriticalSection);

				if (index + 1 != EventTotal)
				{
					for (i = index; i < EventTotal; i++)
					{
						EventArray[i] = EventArray[i + 1];
						SocketArray[i] = SocketArray[i + 1];
					}
				}

				EventTotal--;

				LeaveCriticalSection(&CriticalSection);

				continue;
			}

			// Check to see if the BytesRECV field equals zero. If this is so, then
			// this means a WSARecv call just completed so update the BytesRECV field
			// with the BytesTransferred value from the completed WSARecv() call.

			if (SI->BytesRECV == 0)
			{
				SI->BytesRECV = BytesTransferred;
				SI->BytesSEND = 0;
			}
			else
			{
				SI->BytesSEND += BytesTransferred;
			}

			if (SI->BytesRECV > SI->BytesSEND)
			{
				// Post another WSASend() request.
				// Since WSASend() is not guaranteed to send all of the bytes requested,
				// continue posting WSASend() calls until all received bytes are sent

				ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
				SI->Overlapped.hEvent = EventArray[index];

				SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
				SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

				if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0, &(SI->Overlapped), NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != ERROR_IO_PENDING)
					{
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						continue;
					}
				}

				else
				{
					printf("WSASend() is OK!\n");
				}
			}
			else
			{
				SI->BytesRECV = 0;

				// Now that there are no more bytes to send post another WSARecv() request

				Flags = 0;
				ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

				SI->Overlapped.hEvent = EventArray[index];

				SI->DataBuf.len = BUFFER_SIZE;
				SI->DataBuf.buf = SI->Buffer;

				if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, &(SI->Overlapped), NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != ERROR_IO_PENDING)
					{
						printf("WSARecv() failed with error %d\n", WSAGetLastError());

						continue;
					}
				}
				else
				{
					printf("WSARecv() is OK!\n");
				}
			}
		}
	}
}