#pragma once
#include "net_common.h"
#include "Logger.h"


namespace network
{
	struct SOCKET_INFORMATION
	{
		uint64_t handshakeIn;
		uint64_t handshakeOut;
		uint64_t handshakeResult;
		SOCKET Socket;
	};

	enum class NetworkEvent
	{
		READ,
		WRITE
	};

	struct PER_IO_DATA
	{
		OVERLAPPED Overlapped;
		WSABUF DataBuf;
		CHAR Buffer[BUFFER_SIZE] = { };
		DWORD BytesSEND;
		DWORD BytesRECV;
		NetworkEvent net_event;
	};

	template <typename T>
	class server_interface
	{
	private:
		SOCKET m_listening;
		DWORD m_eventTotal;
		CRITICAL_SECTION CriticalSection;
		bool m_isRunning;
		HANDLE m_CompletionPort;
		std::thread* threads;

		// Array storing all the events
		WSAEVENT m_Events[WSA_MAXIMUM_WAIT_EVENTS];

		// Container for all sockets that has an established connection
		SOCKET_INFORMATION* m_Sockets[WSA_MAXIMUM_WAIT_EVENTS] = { nullptr };

	private:
		DWORD WINAPI ServerWorkerThread();

		SOCKET CreateSocket(const uint16_t& port);
		std::string PrintSocketData(struct addrinfo* p);
		void Listen(const uint32_t& nListen);
		void InitWinsock();
		bool CreateSocketInformation(SOCKET s);
		void ProcessIO();
		void DisconnectClient(SOCKET_INFORMATION*& SI);
		bool ClientIsConnected(SOCKET_INFORMATION* SI);
		void ReadHeader();
		void ReadPayload();
		void WriteHeader();
		void WritePayload();

	public:
		server_interface()
		{
			m_listening = INVALID_SOCKET;

			m_eventTotal = 0;
			m_isRunning = false;
		}
		virtual ~server_interface()
		{
			WSACleanup();
			delete[] threads;
		}

	public:
		void Start(const uint16_t& port);
		void Stop();
		void Broadcast();
		SOCKET WaitForConnection();
		void Send(const SOCKET& socketId, CHAR* buffer, DWORD bytesToSend);
		bool IsRunning();

		// Pure virtuals that happen when an event occurs
		// Called once when a client connects
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) = 0;
		// Called once when a client connects
		virtual void OnClientDisconnect() = 0;
		// Called once when a message is received
		virtual void OnMessageReceived(const SOCKET& socketId, CHAR* buffer, DWORD bytesReceived) = 0;
		// Client has solved the puzzle from the server and is now validated
		virtual void OnClientValidated(const SOCKET& s) = 0;
	};

	template <typename T>
	void server_interface<T>::ReadHeader()
	{

	}

	template <typename T>
	bool server_interface<T>::ClientIsConnected(SOCKET_INFORMATION* SI)
	{
		bool isConnected = false;

		if (SI)
		{
			isConnected = true;
		}

		return isConnected;
	}

	template<typename T>
	void server_interface<T>::DisconnectClient(SOCKET_INFORMATION*& SI)
	{
		closesocket(SI->Socket);
		delete SI;
		SI = nullptr;
	}

	template <typename T>
	bool server_interface<T>::IsRunning()
	{
		return m_isRunning;
	}

	template <typename T>
	bool server_interface<T>::CreateSocketInformation(SOCKET s)
	{
		SOCKET_INFORMATION* SI = new SOCKET_INFORMATION;
		SI->Socket = s;
		SI->handshakeIn = 0;
		SI->handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
		SI->handshakeResult = 0;

		if (CreateIoCompletionPort((HANDLE)SI->Socket, m_CompletionPort, (ULONG_PTR)SI, 0) == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());

			return false;
		}
		else
		{
			LOG_INFO("CreateIoCompletionPort() is OK!");
		}

		PER_IO_DATA* PIO = new PER_IO_DATA;
		ZeroMemory(&PIO->Overlapped, sizeof(OVERLAPPED));
		PIO->BytesSEND = 0;
		PIO->BytesRECV = 0;
		PIO->DataBuf.len = BUFFER_SIZE;
		PIO->DataBuf.buf = PIO->Buffer;
		PIO->net_event = NetworkEvent::READ;

		DWORD Flags = 0;
		DWORD RecvBytes = 0;

		if (WSARecv(SI->Socket, &(PIO->DataBuf), 1, &RecvBytes, &Flags, &(PIO->Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				delete SI;
				delete PIO;
				LOG_ERROR("WSARecv() failed with error %d", WSAGetLastError());
				return false;
			}
			else
			{
				LOG_INFO("Pending receive for client: %lld", SI->Socket);
			}
		}
		else
		{
			LOG_INFO("WSARecv() is OK!");
		}

		return true;
	}

	template <typename T>
	void server_interface<T>::Send(const SOCKET& socketId, CHAR* buffer, DWORD bytesToSend)
	{
		DWORD flags = 0;
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.len = bytesToSend;
		context->DataBuf.buf = buffer;
		DWORD bytesSent = 0;
		context->net_event = NetworkEvent::WRITE;

		if (WSASend(socketId, &context->DataBuf, 1, &bytesSent, flags, (OVERLAPPED*)context, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				delete context;
				LOG_ERROR("WSASend() failed with error %d", WSAGetLastError());
				return;
			}
		}
		else
		{
			LOG_INFO("Sent %d bytes", bytesToSend);
		}
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
			LOG_ERROR("WSAStartup error code: %d", WSAGetLastError());

			return;
		}

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			LOG_ERROR("Could not find a usable version of Winsock.dll");

			return;
		}
	}

	template <typename T>
	void server_interface<T>::Listen(const uint32_t& nListen)
	{
		if (listen(m_listening, nListen) != 0)
		{
			LOG_ERROR("Listen(): failed with error %d", WSAGetLastError());
		}
	}

	template <typename T>
	SOCKET server_interface<T>::WaitForConnection()
	{
		SOCKET clientSocket = WSAAccept(m_listening, nullptr, NULL, NULL, 0);

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
			LOG_ERROR("Addrinfo: %d", WSAGetLastError());
			return INVALID_SOCKET;
		}

		SOCKET listener;

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
#ifdef _DEBUG
			EnterCriticalSection(&CriticalSection);
			LOG_INFO(PrintSocketData(p).c_str());
			LeaveCriticalSection(&CriticalSection);
#endif

			listener = WSASocket(p->ai_family, p->ai_socktype, p->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

			if (listener == INVALID_SOCKET)
			{
				continue;
			}
			if (bind(listener, p->ai_addr, static_cast<int>(p->ai_addrlen)) != 0)
			{
				LOG_ERROR("Bind failed with error: %d", WSAGetLastError());
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

#ifdef _DEBUG
		EnterCriticalSection(&CriticalSection);
		LOG_INFO("[SERVER] Socket created successfully");
		LeaveCriticalSection(&CriticalSection);
#endif

		freeaddrinfo(servinfo);

		return listener;
	}

	template <typename T>
	void server_interface<T>::ProcessIO()
	{
		DWORD index;
		SOCKET currentSocket = INVALID_SOCKET;
		WSANETWORKEVENTS wsaConnectEvents;
		WSANETWORKEVENTS wsaProcessEvents;
	}

	template <typename T>
	void server_interface<T>::Start(const uint16_t& port)
	{
		InitializeCriticalSection(&CriticalSection);
		InitWinsock();
		SYSTEM_INFO SystemInfo;

		// Setup an I/O completion port
		if ((m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());
			return;
		}
		// Determine how many processors are on the system
		GetSystemInfo(&SystemInfo);

		// Create worker threads based on the number of processors available on the
		// system. Create two worker threads for each processor
		threads = new std::thread[SystemInfo.dwNumberOfProcessors * 2];

		for (int i = 0; i < (int)SystemInfo.dwNumberOfProcessors * 2; i++)
		{
			threads[i] = std::thread(&server_interface<T>::ServerWorkerThread, this);
		}

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
		HANDLE acceptEvent = WSACreateEvent();

		if (acceptEvent == WSA_INVALID_EVENT)
		{
			LOG_ERROR("Failed to create event %d", WSAGetLastError());
			return;
		}

		WSAEventSelect(m_listening, acceptEvent, FD_ACCEPT);

		DWORD ThreadId = 0;

		m_isRunning = true;

		while (1)
		{
			WSANETWORKEVENTS netEvents;

			// Will put thread to sleep unless there is I/O to process or a new connection has been made
			if ((WSAWaitForMultipleEvents(1, &acceptEvent, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
			{
				LOG_ERROR("WSAWaitForEvents failed with code: %d", WSAGetLastError());
				continue;
			}

			WSAEnumNetworkEvents(m_listening, acceptEvent, &netEvents);

			if (netEvents.lNetworkEvents & FD_ACCEPT && (netEvents.iErrorCode[FD_ACCEPT_BIT] == 0))
			{
				SOCKET clientSocket = WaitForConnection();

				struct sockaddr_in c = {};
				socklen_t cLen = sizeof(c);
				getpeername(clientSocket, (struct sockaddr*)&c, &cLen);
				char ipAsString[IPV6_ADDRSTRLEN] = {};
				inet_ntop(c.sin_family, &c.sin_addr, ipAsString, sizeof(ipAsString));
				uint16_t port = GetPort((struct sockaddr*)&c);

				//this->ClientConnectHandler(ipAsString, port);
				OnClientConnect(ipAsString, port);

				EnterCriticalSection(&CriticalSection);
				CreateSocketInformation(clientSocket);
				LeaveCriticalSection(&CriticalSection);
			}
		}
	}

	template <typename T>
	void server_interface<T>::Stop()
	{
		EnterCriticalSection(&CriticalSection);
		m_isRunning = false;
		LeaveCriticalSection(&CriticalSection);
	}

	template <typename T>
	DWORD server_interface<T>::ServerWorkerThread()
	{
		DWORD BytesTransferred = 0;
		SOCKET_INFORMATION* PER_HANDLE_DATA = nullptr;
		PER_IO_DATA* io_context = nullptr;
		DWORD bytesReceived = 0;
		LPOVERLAPPED lpOverlapped;
		BOOL bResult = false;
		BOOL shouldDisconnect = false;

		while (1)
		{
			bResult = GetQueuedCompletionStatus(m_CompletionPort, &BytesTransferred, (PULONG_PTR)&PER_HANDLE_DATA, &lpOverlapped, INFINITE);

			// Failed but still filled in data into lpOverlapped, need to de-allocate.
			if (!bResult && lpOverlapped != NULL)
			{
				shouldDisconnect = true;
			}
			// Failed and didnt fill in data
			else if (!bResult && lpOverlapped == NULL)
			{
				LOG_ERROR("GetQueuedCompletionStatus() failed with error: %d", GetLastError());
				continue;
			}
			// If an I/O was completed but we received no data means a client must've disconnected
			if (BytesTransferred == 0)
			{
				shouldDisconnect = true;
			}

			io_context = CONTAINING_RECORD(lpOverlapped, PER_IO_DATA, PER_IO_DATA::Overlapped);

			if (shouldDisconnect)
			{
				delete io_context;
				io_context = nullptr;
				DisconnectClient(PER_HANDLE_DATA);
				OnClientDisconnect();
				shouldDisconnect = false;
				continue;
			}

			EnterCriticalSection(&CriticalSection);
			LOG_INFO("GetQueuedCompletionStatus() is OK!");
			LeaveCriticalSection(&CriticalSection);

			// Receive call has completed
			if (io_context->net_event == NetworkEvent::READ)
			{
				EnterCriticalSection(&CriticalSection);
				LOG_INFO("OP: READ");
				LOG_INFO("Thread ID: %ld", GetCurrentThreadId());
				this->OnMessageReceived(PER_HANDLE_DATA->Socket, io_context->DataBuf.buf, BytesTransferred);
				LeaveCriticalSection(&CriticalSection);
				PER_IO_DATA* newContext = new PER_IO_DATA;
				DWORD flags = 0;
				ZeroMemory(&newContext->Overlapped, sizeof(OVERLAPPED));
				newContext->DataBuf.len = BUFFER_SIZE;
				newContext->DataBuf.buf = newContext->Buffer;
				bytesReceived = 0;
				newContext->net_event = NetworkEvent::READ;
				newContext->BytesRECV = 0;
				newContext->BytesSEND = 0;

				// Prime the completion status queue with a pending READ request
				if (WSARecv(PER_HANDLE_DATA->Socket, &newContext->DataBuf, 1, &bytesReceived, &flags, (OVERLAPPED*)newContext, NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != ERROR_IO_PENDING)
					{
						LOG_ERROR("WSARecv() failed with error %d", WSAGetLastError());
						return 0;
					}
					else
					{
						EnterCriticalSection(&CriticalSection);
						LOG_INFO("Pending receive call for client: %lld", PER_HANDLE_DATA->Socket);
						LeaveCriticalSection(&CriticalSection);
					}
				}
			}
			// Send call has completed
			else if (io_context->net_event == NetworkEvent::WRITE)
			{
				EnterCriticalSection(&CriticalSection);
				LOG_INFO("OP: WRITE");
				LeaveCriticalSection(&CriticalSection);
			}
			// Since PER_IO_DATA is created for EVERY I/O that comes in we need to 
			// delete that data struct after I/O has completed
			if (io_context)
			{
				EnterCriticalSection(&CriticalSection);
				LOG_INFO("Deleting current context! (This must happen)");
				LeaveCriticalSection(&CriticalSection);
				delete io_context;
				io_context = nullptr;
			}
		}
		return 0;
	}
}