#pragma once
#include "net_tsqueue.h"
//#define PRINT_NETWORK_DEBUG

namespace network
{
	// What current state are the current connection in
	enum class NetState
	{
		WRITE_VALIDATION,
		READ_VALIDATION,
		READ_HEADER,
		WRITE_HEADER,
		READ_PAYLOAD,
		WRITE_PAYLOAD
	};

	// Information regarding every connection
	struct SOCKET_INFORMATION
	{
		uint64_t handshakeIn;
		uint64_t handshakeOut;
		uint64_t handshakeResult;
		SOCKET Socket;
	};

	// Information regarding every input or output
	struct PER_IO_DATA
	{
		OVERLAPPED Overlapped = {};
		WSABUF DataBuf = {};
		CHAR Buffer[BUFFER_SIZE] = { };
		DWORD BytesSEND = 0;
		DWORD BytesRECV = 0;
		NetState net_state;
	};

	template <typename T>
	class server_interface
	{
	private:
		SOCKET m_listening;
		bool m_isRunning;
		HANDLE m_CompletionPort;
		message<T> msgTemp;

	protected:
		tsQueue<T> m_messagesIn;
		CRITICAL_SECTION lock;

	private:
		static void WINAPI ReadValidationDone(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
		DWORD WINAPI ServerWorkerThread();
		DWORD WINAPI ProcessIncomingConnections(LPVOID param);

		SOCKET CreateSocket(const uint16_t& port);
		std::string PrintSocketData(struct addrinfo* p);
		void Listen(const uint32_t& nListen);
		void InitWinsock();
		bool CreateSocketInformation(SOCKET s);
		void DisconnectClient(SOCKET_INFORMATION*& SI);
		bool ClientIsConnected(SOCKET_INFORMATION* SI);

	public:
		server_interface()
		{
			m_listening = INVALID_SOCKET;
			m_isRunning = false;
			lock = {};
			m_CompletionPort = {};
		}
		server_interface<T>& operator=(const server_interface<T>& other) = delete;
		server_interface(const server_interface<T>& other) = delete;

		virtual ~server_interface()
		{
			WSACleanup();
			DeleteCriticalSection(&lock);
		}

	public:
		void Start(const uint16_t& port);
		void Stop();
		void Broadcast(message<T>& msg);
		SOCKET WaitForConnection();
		void Send(const SOCKET& socketId, message<T>& msg);
		bool IsRunning();

		void WriteValidation(SOCKET_INFORMATION* SI, uint64_t handshakeOut);
		void ReadValidation(PER_IO_DATA* context, SOCKET_INFORMATION* SI);
		void ReadHeader(PER_IO_DATA* context, SOCKET_INFORMATION* SI);
		void ReadPayload(PER_IO_DATA* context, SOCKET_INFORMATION* SI);
		void WriteHeader(SOCKET_INFORMATION* SI);
		void WritePayload();
		void PrimeReadHeader(SOCKET_INFORMATION* SI);
		void PrimeReadPayload(SOCKET_INFORMATION* SI, size_t size);
		void PrimeReadValidation(SOCKET_INFORMATION* SI);

		// Pure virtuals that happen when an event occurs
		// Called once when a client connects
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) = 0;
		// Called once when a client connects
		virtual void OnClientDisconnect() = 0;
		// Called once when a message is received
		virtual void OnMessageReceived(const SOCKET& socketId, message<T>& msg) = 0;
		// Client has solved the puzzle from the server and is now validated
		virtual void OnClientValidated(const SOCKET& s) = 0;
	};

	template <typename T>
	void server_interface<T>::PrimeReadValidation(SOCKET_INFORMATION* SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = context->Buffer;
		context->DataBuf.len = sizeof(uint64_t);
		context->net_state = NetState::READ_VALIDATION;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->Socket, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("Error sending puzzle with error: %ld", GetLastError());
				return;
			}
		}
	}

	template <typename T>
	void server_interface<T>::PrimeReadHeader(SOCKET_INFORMATION* SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = context->Buffer;
		context->DataBuf.len = sizeof(msg_header<T>);
		context->net_state = NetState::READ_HEADER;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->Socket, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				delete SI;
				LOG_ERROR("Error sending puzzle with error: %ld", GetLastError());
				return;
			}
		}
	}

	template <typename T>
	void server_interface<T>::PrimeReadPayload(SOCKET_INFORMATION* SI, size_t size)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = context->Buffer;
		context->DataBuf.len = (ULONG)size;
		context->net_state = NetState::READ_PAYLOAD;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->Socket, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete SI;
				delete context;
				LOG_ERROR("Error sending puzzle with error: %ld", GetLastError());
				return;
			}
		}
	}

	template <typename T>
	void server_interface<T>::ReadPayload(PER_IO_DATA* context, SOCKET_INFORMATION* SI)
	{
		msgTemp.payload.resize(size_t(context->DataBuf.len));
		memcpy(msgTemp.payload.data(), context->Buffer, context->DataBuf.len);

		this->OnMessageReceived(SI->Socket, msgTemp);
	}

	template <typename T>
	void server_interface<T>::ReadHeader(PER_IO_DATA* context, SOCKET_INFORMATION* SI)
	{
		ZeroMemory(&msgTemp.header, sizeof(msg_header<T>));
		msgTemp.payload.clear();
		memcpy(&msgTemp.header, context->Buffer, sizeof(msg_header<T>));

		if (msgTemp.header.size > 0)
		{
			this->PrimeReadPayload(SI, msgTemp.size() - sizeof(msg_header<T>));
		}
		else
		{
			this->OnMessageReceived(SI->Socket, msgTemp);
		}
	}

	template <typename T>
	void server_interface<T>::WriteHeader(SOCKET_INFORMATION* SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = context->Buffer;
		context->DataBuf.len = sizeof(msg_header<T>);
		context->net_state = NetState::WRITE_HEADER;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSASend(SI->Socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete SI;
				delete context;
				LOG_ERROR("Error sending puzzle with error: %ld", GetLastError());
				return false;
			}
		}
	}

	template <typename T>
	void server_interface<T>::ReadValidation(PER_IO_DATA* context, SOCKET_INFORMATION* SI)
	{
		memcpy(&SI->handshakeIn, context->DataBuf.buf, sizeof(uint64_t));

		if (SI->handshakeIn == SI->handshakeResult)
		{
			EnterCriticalSection(&lock);
			this->OnClientValidated(SI->Socket);
			LeaveCriticalSection(&lock);
		}
		else
		{
			EnterCriticalSection(&lock);
			this->DisconnectClient(SI);
			LOG_NETWORK("Client failed validation");
			LeaveCriticalSection(&lock);
		}
	}

	template <typename T>
	void server_interface<T>::WriteValidation(SOCKET_INFORMATION* SI, uint64_t handshakeOut)
	{
		// Sends a puzzle that the client has to solve to be accepted into the server
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		memcpy(context->Buffer, &handshakeOut, sizeof(uint64_t));
		context->DataBuf.buf = context->Buffer;
		context->DataBuf.len = sizeof(uint64_t);
		context->net_state = NetState::WRITE_VALIDATION;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(SI->Socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("Error sending puzzle with error: %ld", GetLastError());
			}
		}
	}

	template <typename T>
	DWORD server_interface<T>::ProcessIncomingConnections(LPVOID param)
	{
		HANDLE eventHandle = param;
		while (m_isRunning)
		{
			WSANETWORKEVENTS netEvents;
			DWORD success = 0;

			// Will put thread to sleep unless there is I/O to process or a new connection has been made
			if ((success = WSAWaitForMultipleEvents(1, &eventHandle, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
			{
				LOG_ERROR("WSAWaitForEvents failed with code: %d", WSAGetLastError());
				continue;
			}
			if (success == WSA_WAIT_TIMEOUT)
			{
				LOG_NETWORK("Timed out!");
				continue;
			}

			WSAEnumNetworkEvents(m_listening, eventHandle, &netEvents);

			if (netEvents.lNetworkEvents & FD_ACCEPT && (netEvents.iErrorCode[FD_ACCEPT_BIT] == 0))
			{
				SOCKET clientSocket = WaitForConnection();

				struct sockaddr_in c = {};
				socklen_t cLen = sizeof(c);
				getpeername(clientSocket, (struct sockaddr*)&c, &cLen);
				char ipAsString[IPV6_ADDRSTRLEN] = {};
				inet_ntop(c.sin_family, &c.sin_addr, ipAsString, sizeof(ipAsString));
				uint16_t port = GetPort((struct sockaddr*)&c);

				OnClientConnect(ipAsString, port);

				CreateSocketInformation(clientSocket);
			}
		}
		return 0;
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

		this->OnClientDisconnect();
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
		SI->handshakeResult = scrambleData(SI->handshakeOut);

		if (CreateIoCompletionPort((HANDLE)SI->Socket, m_CompletionPort, (ULONG_PTR)SI, 0) == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());

			return false;
		}

		WriteValidation(SI, SI->handshakeOut);

		return true;
	}

	template <typename T>
	void server_interface<T>::Send(const SOCKET& socketId, message<T>& msg)
	{
		DWORD flags = 0;
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		ULONG bytesToSend = sizeof(msg_header<T>);
		memcpy(&context->Buffer[0], &msg.header, bytesToSend);
		context->DataBuf.buf = context->Buffer;
		context->DataBuf.len = bytesToSend;
		DWORD bytesSent = 0;
		context->net_state = NetState::WRITE_HEADER;

		if (WSASend(socketId, &context->DataBuf, 1, &bytesSent, flags, (OVERLAPPED*)context, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				delete context;
				LOG_ERROR("WSASend() failed with error %d", WSAGetLastError());
				return;
			}
		}

		if (msg.header.size > 0)
		{
			PER_IO_DATA* context2 = new PER_IO_DATA;
			ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
			context2->DataBuf.len = ULONG(msg.payload.size());
			memcpy(&context2->Buffer[0], &msg.payload[0], msg.payload.size());
			context2->DataBuf.buf = context2->Buffer;
			context2->net_state = NetState::WRITE_PAYLOAD;
			DWORD bytesSent = 0;

			if (WSASend(socketId, &context2->DataBuf, 1, &bytesSent, flags, (OVERLAPPED*)context2, NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					delete context2;
					LOG_ERROR("WSASend() failed with error %d", WSAGetLastError());
					return;
				}
			}
		}
	}

	template <typename T>
	void server_interface<T>::Broadcast(message<T>& msg)
	{
		//for (auto& c : connections)
		//{
		//	Send(c.second, buffer, bytesToSend);
		//}
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
			EnterCriticalSection(&lock);
			LOG_NETWORK(PrintSocketData(p).c_str());
			LeaveCriticalSection(&lock);
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

		freeaddrinfo(servinfo);

		return listener;
	}

	template <typename T>
	void server_interface<T>::Start(const uint16_t& port)
	{
		InitializeCriticalSection(&lock);
		InitWinsock();
		SYSTEM_INFO SystemInfo;
		m_isRunning = true;

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
		for (size_t i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
		{
			std::thread workerThread = std::thread(&server_interface<T>::ServerWorkerThread, this);
			workerThread.detach();
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

		std::thread t(&server_interface<T>::ProcessIncomingConnections, this, acceptEvent);
		t.detach();
	}

	template <typename T>
	void server_interface<T>::Stop()
	{
		EnterCriticalSection(&lock);
		m_isRunning = false;
		LeaveCriticalSection(&lock);
	}

	template <typename T>
	DWORD server_interface<T>::ServerWorkerThread()
	{
		DWORD BytesTransferred = 0;
		SOCKET_INFORMATION* SI = nullptr;
		PER_IO_DATA* context = nullptr;
		DWORD bytesReceived = 0;
		LPOVERLAPPED lpOverlapped;
		BOOL bResult = false;
		BOOL shouldDisconnect;;

		while (m_isRunning)
		{
			shouldDisconnect = false;
			bResult = GetQueuedCompletionStatus(m_CompletionPort, &BytesTransferred, (PULONG_PTR)&SI, &lpOverlapped, INFINITE);

			// Failed but allocated data for lpOverlapped, need to de-allocate.
			if (!bResult && lpOverlapped != NULL)
			{
				shouldDisconnect = true;
			}
			// Failed and lpOverlapped was not allocated any data
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
			// Basically a memcpy so we have data in correct structure
			context = CONTAINING_RECORD(lpOverlapped, PER_IO_DATA, PER_IO_DATA::Overlapped);

			if (shouldDisconnect)
			{
				this->DisconnectClient(SI);
				delete context;
				continue;
			}

			switch (context->net_state)
			{
			case NetState::READ_HEADER:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_INFO("Read I/O completed! \"HEADER\"");
				LeaveCriticalSection(&lock);
#endif
				this->ReadHeader(context, SI);
				break;
			}
			case NetState::READ_PAYLOAD:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_INFO("Read I/O completed! \"PAYLOAD\"");
				LeaveCriticalSection(&lock);
#endif
				this->ReadPayload(context, SI);
				break;
			}
			case NetState::READ_VALIDATION:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_INFO("Read I/O completed! \"VALIDATION\"");
				LeaveCriticalSection(&lock);
#endif
				this->ReadValidation(context, SI);
				break;
			}
			case NetState::WRITE_HEADER:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_INFO("Write I/O completed! \"HEADER\"");
				LeaveCriticalSection(&lock);
#endif
				this->PrimeReadHeader(SI);
				break;
			}
			case NetState::WRITE_PAYLOAD:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_INFO("Write I/O completed! \"PAYLOAD\"");
				LeaveCriticalSection(&lock);
#endif
				break;
			}
			case NetState::WRITE_VALIDATION:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_INFO("Write I/O completed! \"VALIDATION\"");
				LeaveCriticalSection(&lock);
#endif
				this->PrimeReadValidation(SI);
				break;
			}
			}
			// Since PER_IO_DATA is created for EVERY I/O that comes in we need to 
			// delete that data struct after I/O has completed
			if (context)
			{
				delete context;
				context = nullptr;
			}
		}
		return 0;
	}
}