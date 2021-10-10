#pragma once
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_common.h"
#include <future>
#define PRINT_NETWORK_DEBUG

namespace network
{
	template <typename T>
	class server_interface
	{
	private:
		// Information regarding every connection
		struct SOCKET_INFORMATION
		{
			uint64_t handshakeIn = 0;
			uint64_t handshakeOut = 0;
			uint64_t handshakeResult = 0;
			SOCKET Socket = {};
			message<T> msgTempIn = {};
		};

		SOCKET m_listening;
		bool m_isRunning;
		HANDLE m_CompletionPort;
		std::thread* workerThreads;
		std::thread acceptThread;
		int nrOfThreads;
		tsQueue<owned_message<T>> m_qMessagesOut;

	protected:
		std::unordered_map<uint32_t, SOCKET> connections;
		std::function<void(message<T>&)> messageReceivedHandler;
		CRITICAL_SECTION lock;
		tsQueue<message<T>> m_qMessagesIn;

	protected:
		// Called once when a client connects
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) = 0;
		// Called once when a client connects
		virtual void OnClientDisconnect() = 0;
		// Called once when a message is received
		virtual void OnMessageReceived(message<T>& msg) = 0;
		// Client has solved the puzzle from the server and is now validated
		virtual void OnClientValidated(const SOCKET& socket) = 0;

	private:
		DWORD WINAPI ServerWorkerThread();
		DWORD WINAPI ProcessIncomingConnections(LPVOID param);

		SOCKET CreateSocket(const uint16_t& port);
		std::string PrintSocketData(struct addrinfo* p);
		bool Listen(const uint32_t& nListen);
		void InitWinsock();
		bool CreateSocketInformation(SOCKET& s);
		void DisconnectClient(SOCKET_INFORMATION*& SI);
		bool ClientIsConnected(const SOCKET& socket);
		SOCKET WaitForConnection();

		// FUNCTIONS TO EASIER HANDLE DATA IN AND OUT FROM SERVER
		void WriteValidation(const SOCKET& socketId, uint64_t handshakeOut);
		void ReadValidation(SOCKET_INFORMATION*& SI, PER_IO_DATA* context);
		void ReadHeader(SOCKET_INFORMATION*& SI, PER_IO_DATA* context);
		void ReadPayload(SOCKET_INFORMATION*& SI, PER_IO_DATA* context);
		void WriteMessage();
		void PrimeReadHeader(SOCKET_INFORMATION*& SI);
		void PrimeReadPayload(SOCKET_INFORMATION*& SI);
		void PrimeReadValidation(SOCKET_INFORMATION*& SI);

		static void AlertThread();
		static void CALLBACK AsyncWriteMessage(ULONG_PTR param);

	public:
		// Constructor and Deconstructor
		server_interface() = default;
		server_interface(std::function<void(message<T>&)> handler)
			:messageReceivedHandler(handler)
		{
			m_listening = INVALID_SOCKET;
			m_isRunning = false;
			lock = {};
			m_CompletionPort = {};
			workerThreads = nullptr;
			nrOfThreads = 0;
		}
		server_interface<T>& operator=(const server_interface<T>& other) = delete;
		server_interface(const server_interface<T>& other) = delete;

		virtual ~server_interface()
		{
			WSACleanup();
			DeleteCriticalSection(&lock);
			delete[] workerThreads;
		}

	public:
		bool Start(const uint16_t& port);
		void Stop();
		void Broadcast(message<T>& msg);
		void SendToClient(const SOCKET& socket, message<T>& msg);
		bool IsRunning();
		bool isClientConnected(const SOCKET& socket)const;
	};

	template <typename T>
	bool server_interface<T>::isClientConnected(const SOCKET& socket)const
	{
		bool isConnected = false;

		if (socket != INVALID_SOCKET)
		{
			isConnected = true;
		}
		return isConnected;
	}

	template <typename T>
	void server_interface<T>::AsyncWriteMessage(ULONG_PTR param)
	{
		server_interface<T>* s = (server_interface<T>*)param;
		s->WriteMessage();
	}

	template <typename T>
	void server_interface<T>::AlertThread()
	{
		LOG_INFO("Thread id: %lu alerted!", GetCurrentThreadId());
	}

	template <typename T>
	void server_interface<T>::PrimeReadValidation(SOCKET_INFORMATION*& SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&SI->handshakeIn;
		context->DataBuf.len = sizeof(uint64_t);
		context->state = NetState::READ_VALIDATION;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->Socket, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("Error sending puzzle with error: %ld", GetLastError());
			}
		}
	}

	template <typename T>
	void server_interface<T>::PrimeReadHeader(SOCKET_INFORMATION*& SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&SI->msgTempIn.header;
		context->DataBuf.len = sizeof(msg_header<T>);
		context->state = NetState::READ_HEADER;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->Socket, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("WSARecv header with error: %ld", GetLastError());
			}
		}
	}

	template <typename T>
	void server_interface<T>::PrimeReadPayload(SOCKET_INFORMATION*& SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		size_t size = SI->msgTempIn.header.size - sizeof(msg_header<T>);
		SI->msgTempIn.payload.resize(size);
		context->DataBuf.buf = (CHAR*)SI->msgTempIn.payload.data();
		context->DataBuf.len = (ULONG)size;
		context->state = NetState::READ_PAYLOAD;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->Socket, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("WSARecv payload with error: %ld", GetLastError());
			}
		}
	}

	template <typename T>
	void server_interface<T>::ReadPayload(SOCKET_INFORMATION*& SI, PER_IO_DATA* context)
	{
		this->m_qMessagesIn.push_back(SI->msgTempIn);
		SI->msgTempIn.payload.clear();
		this->PrimeReadHeader(SI);
	}

	template <typename T>
	void server_interface<T>::ReadHeader(SOCKET_INFORMATION*& SI, PER_IO_DATA* context)
	{
		if (SI->msgTempIn.header.size > 0)
		{
			if (SI->msgTempIn.header.size > 3000)
			{
				LOG_ERROR("Allocating to much memory!");
			}
			this->PrimeReadPayload(SI);
		}
		else
		{
			this->m_qMessagesIn.push_back(SI->msgTempIn);
			ZeroMemory(&SI->msgTempIn.header, sizeof(msg_header<T>));
			this->PrimeReadHeader(SI);
		}
	}

	template <typename T>
	void server_interface<T>::WriteMessage()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		char buffer[BUFFER_SIZE] = {};
		owned_message<T> msg = m_qMessagesOut.front();
		memcpy(&buffer[0], &msg.msg.header, sizeof(msg_header<T>));
		if (msg.msg.header.size > 0)
		{
			memcpy(&buffer[sizeof(msg_header<T>)], msg.msg.payload.data(), msg.msg.payload.size());
		}
		context->DataBuf.buf = (CHAR*)buffer;
		context->DataBuf.len = ULONG(sizeof(msg_header<T>) + msg.msg.payload.size());
		context->state = NetState::WRITE_MESSAGE;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(msg.remote, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("WSASend message with error: %ld", GetLastError());
			}
		}
	}

	template <typename T>
	void server_interface<T>::ReadValidation(SOCKET_INFORMATION*& SI, PER_IO_DATA* context)
	{
		memcpy(&SI->handshakeIn, context->DataBuf.buf, sizeof(uint64_t));

		if (SI->handshakeIn == SI->handshakeResult)
		{
			EnterCriticalSection(&lock);
			this->OnClientValidated(SI->Socket);
			LeaveCriticalSection(&lock);
			this->PrimeReadHeader(SI);
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
	void server_interface<T>::WriteValidation(const SOCKET& socketId, uint64_t handshakeOut)
	{
		// Sends a puzzle that the client has to solve to be accepted into the server
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&handshakeOut;
		context->DataBuf.len = sizeof(uint64_t);
		context->state = NetState::WRITE_VALIDATION;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(socketId, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
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
			if ((success = WSAWaitForMultipleEvents(1, &eventHandle, FALSE, WSA_INFINITE, TRUE)) == WSA_WAIT_FAILED)
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
	bool server_interface<T>::ClientIsConnected(const SOCKET& socket)
	{
		EnterCriticalSection(&lock);
		bool isConnected = false;

		if (socket != INVALID_SOCKET)
		{
			isConnected = true;
		}
		LeaveCriticalSection(&lock);

		return isConnected;
	}

	template<typename T>
	void server_interface<T>::DisconnectClient(SOCKET_INFORMATION*& SI)
	{
		EnterCriticalSection(&lock);
		if (SI != NULL)
		{
			closesocket(SI->Socket);
			delete SI;
			SI = nullptr;
			this->OnClientDisconnect();
		}
		LeaveCriticalSection(&lock);
	}

	template <typename T>
	bool server_interface<T>::IsRunning()
	{
		return m_isRunning;
	}

	template <typename T>
	bool server_interface<T>::CreateSocketInformation(SOCKET& s)
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

		this->WriteValidation(SI->Socket, SI->handshakeOut);

		return true;
	}

	template <typename T>
	void server_interface<T>::SendToClient(const SOCKET& socket, message<T>& msg)
	{
		if (ClientIsConnected(socket))
		{
			owned_message<T> message;
			message.msg = msg;
			message.remote = socket;
			EnterCriticalSection(&lock);
			bool writingMessage = !m_qMessagesOut.empty();
			m_qMessagesOut.push_back(message);
			if (!writingMessage)
			{
				int threadID = rand() % nrOfThreads;
				QueueUserAPC((PAPCFUNC)&server_interface<T>::AsyncWriteMessage, workerThreads[threadID].native_handle(), (ULONG_PTR)this);
			}
			LeaveCriticalSection(&lock);
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
	bool server_interface<T>::Listen(const uint32_t& nListen)
	{
		if (listen(m_listening, nListen) != 0)
		{
			LOG_ERROR("Listen(): failed with error %d", WSAGetLastError());
			return false;
		}

		return true;
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
				listener = INVALID_SOCKET;
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
	bool server_interface<T>::Start(const uint16_t& port)
	{
		InitializeCriticalSection(&lock);
		InitWinsock();
		SYSTEM_INFO SystemInfo;
		if ((m_listening = CreateSocket(port)) == INVALID_SOCKET)
		{
			return false;
		}

		// Setup an I/O completion port
		if ((m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());
			return false;
		}

		// Options to disable Nagle's algorithm (can queue up multiple packets instead of sending 1 by 1)
		// SO_REUSEADDR will let the server to reuse the port its bound on even if it have not closed 
		// by the the operating system yet.
		int enable = 1;
		if (setsockopt(m_listening, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(int)) != 0)
		{
			LOG_ERROR("setsockopt: %d", WSAGetLastError());
			return false;
		}

		if (setsockopt(m_listening, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int)) != 0)
		{
			LOG_ERROR("setsockopt: %d", WSAGetLastError());
			return false;
		}

		if (!this->Listen(SOMAXCONN))
		{
			return false;
		}

		// Create an event for the accepting socket
		HANDLE acceptEvent = WSACreateEvent();

		if (acceptEvent == WSA_INVALID_EVENT)
		{
			LOG_ERROR("Failed to create event %d", WSAGetLastError());
			return false;
		}
		WSAEventSelect(m_listening, acceptEvent, FD_ACCEPT);

		acceptThread = std::thread(&server_interface<T>::ProcessIncomingConnections, this, acceptEvent);

		// Determine how many processors are on the system
		GetSystemInfo(&SystemInfo);
		nrOfThreads = (int)SystemInfo.dwNumberOfProcessors;
		workerThreads = new std::thread[nrOfThreads];

		m_isRunning = true;
		// Create worker threads based on the number of processors available on the
		// system. Create two worker threads for each processor
		for (size_t i = 0; i < nrOfThreads; i++)
		{
			workerThreads[i] = std::thread(&server_interface<T>::ServerWorkerThread, this);
		}

		return true;
	}

	template <typename T>
	void server_interface<T>::Stop()
	{
		EnterCriticalSection(&lock);
		m_isRunning = false;

		if (m_listening != INVALID_SOCKET)
		{
			SYSTEM_INFO SystemInfo;
			GetSystemInfo(&SystemInfo);

			for (auto con : connections)
			{
				if (CancelIoEx((HANDLE)con.second, NULL))
				{
					LOG_INFO("Cancelled all IO");
				}
				else
				{
					LOG_INFO("Didnt cancel IO: %d", GetLastError());
				}
			}
			LeaveCriticalSection(&lock);
			for (int i = 0; i < nrOfThreads; i++)
			{
				QueueUserAPC((PAPCFUNC)server_interface<T>::AlertThread, (HANDLE)workerThreads[i].native_handle(), NULL);
				workerThreads[i].join();
			}

			QueueUserAPC((PAPCFUNC)server_interface<T>::AlertThread, (HANDLE)acceptThread.native_handle(), NULL);
			acceptThread.join();
			return;
		}
		LeaveCriticalSection(&lock);
	}

	template <typename T>
	DWORD server_interface<T>::ServerWorkerThread()
	{
		DWORD BytesTransferred = 0;
		SOCKET_INFORMATION* SI = nullptr;
		PER_IO_DATA* context;
		DWORD bytesReceived = 0;
		const DWORD CAP = 1;
		OVERLAPPED_ENTRY Entries[CAP];
		ULONG EntriesRemoved = 0;
		BOOL ShouldShutdown = false;

		while (m_isRunning)
		{
			SOCKET_INFORMATION* SI = nullptr;
			memset(Entries, 0, sizeof(Entries));
			if (!GetQueuedCompletionStatusEx(m_CompletionPort, Entries, CAP, &EntriesRemoved, WSA_INFINITE, TRUE))
			{
				DWORD LastError = GetLastError();
				if (LastError != ERROR_EXE_MARKED_INVALID)
				{
					LOG_ERROR("%d", LastError);
				}
			}

			for (int i = 0; i < (int)EntriesRemoved; i++)
			{
				if (Entries[i].lpOverlapped != NULL)
				{
					SI = (SOCKET_INFORMATION*)Entries[i].lpCompletionKey;
					context = (PER_IO_DATA*)Entries[i].lpOverlapped;
					if (Entries[i].dwNumberOfBytesTransferred == 0)
					{
						this->DisconnectClient(SI);
						delete context;
						continue;
					}
					// If an I/O was completed but we received no data means a client must've disconnected
					// Basically a memcpy so we have data in correct structure
					// I/O has completed, process it
					if (HasOverlappedIoCompleted(Entries[i].lpOverlapped))
					{
						switch (context->state)
						{
						case NetState::READ_HEADER:
						{
							this->ReadHeader(SI, context);
							break;
						}
						case NetState::READ_PAYLOAD:
						{
							this->ReadPayload(SI, context);
							break;
						}
						case NetState::READ_VALIDATION:
						{
							this->ReadValidation(SI, context);
							break;
						}
						case NetState::WRITE_MESSAGE:
						{
							EnterCriticalSection(&lock);
							m_qMessagesOut.pop_front();

							if (!m_qMessagesOut.empty())
							{
								this->WriteMessage();
							}
							LeaveCriticalSection(&lock);
							break;
						}
						case NetState::WRITE_VALIDATION:
						{
							this->PrimeReadValidation(SI);
							break;
						}
						}
						// Since PER_IO_DATA is created with new for EVERY I/O we need to 
						// de-allocate that data after I/O has completed
						if (context)
						{
							delete context;
							context = nullptr;
						}
					}
				}
			}
		}
		return 0;
	}
}