#pragma once
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_common.h"

namespace network
{
	template <typename T>
	class server_interface
	{
	private:
		// Information regarding every connection
		SOCKET m_listening;
		SOCKET m_udpSocket;
		bool m_isRunning;
		HANDLE m_CompletionPort;
		HANDLE m_udpHandle;
		std::thread* m_workerThreads;
		std::thread m_acceptThread;
		int m_nrOfThreads;
		tsQueue<owned_message<T>> m_qMessagesOut;
		tsQueue<owned_message<T>> m_qPrioMessagesOut;
		CRITICAL_SECTION udpLock;

	protected:
		std::unordered_map<uint32_t, SOCKET> m_connections;
		std::unordered_map<SOCKET, SOCKET_INFORMATION<T>*> m_socketInfo;
		std::function<void(message<T>&)> messageReceivedHandler;
		CRITICAL_SECTION lock;
		tsQueue<message<T>> m_qMessagesIn;
		tsQueue<message<T>> m_qPrioMessagesIn;

	protected:
		// Called once when a client connects
		virtual void OnClientConnect(std::string&& ip, const uint16_t& port) = 0;
		// Called once when a client connects
		virtual void OnClientDisconnect(const SOCKET& socket) = 0;
		// Called once when a message is received
		virtual void OnMessageReceived(message<T>& msg) = 0;
		// Client has solved the puzzle from the server and is now validated
		virtual void OnClientValidated(const SOCKET& socket) = 0;

	private:
		DWORD WINAPI ProcessTCPIO();
		DWORD WINAPI ProcessUDPIO();
		DWORD WINAPI ProcessIncomingConnections(LPVOID param);

		SOCKET CreateSocket(const uint16_t& port, SockType&& type);
		std::string PrintSocketData(struct addrinfo* p);
		bool Listen(const uint32_t& nListen);
		void InitWinsock();
		bool CreateSocketInformation(const SOCKET& s);
		void DisconnectClient(SOCKET_INFORMATION<T>* SI);
		SOCKET WaitForConnection();
		bool IsConnected(const Socket_t& socket)const;

		// FUNCTIONS TO EASIER HANDLE DATA IN AND OUT FROM SERVER
		void WriteValidation(const SOCKET& socketId, uint64_t handshakeOut);
		void ReadValidation(SOCKET_INFORMATION<T>*& SI, PER_IO_DATA* context);
		void ReadHeader(SOCKET_INFORMATION<T>*& SI, PER_IO_DATA* context);
		void ReadPayload(SOCKET_INFORMATION<T>*& SI, PER_IO_DATA* context);
		void WriteHeader();
		void WritePacket();
		void WritePayload();
		void PrimeReadHeader(SOCKET_INFORMATION<T>* SI);
		void PrimeReadPayload(SOCKET_INFORMATION<T>* SI);
		void PrimeReadValidation(SOCKET_INFORMATION<T>* SI);
		void PrimeReadPacket(SOCKET_INFORMATION<T>* SI);

		static void AlertThread();

	public:
		server_interface() = default;
		server_interface(std::function<void(message<T>&)> handler)
			:messageReceivedHandler(handler)
		{
			m_listening = INVALID_SOCKET;
			m_isRunning = false;
			lock = {};
			m_CompletionPort = {};
			m_workerThreads = nullptr;
			m_nrOfThreads = 0;
		}
		server_interface<T>& operator=(const server_interface<T>& other) = delete;
		server_interface(const server_interface<T>& other) = delete;

		virtual ~server_interface()
		{
			WSACleanup();
			DeleteCriticalSection(&lock);
			DeleteCriticalSection(&udpLock);
			delete[] m_workerThreads;
		}

	public:
		bool Start(const uint16_t& port);
		void Stop();
		void SendToClient(const uint32_t& id, message<T>& msg);
		void SendToClientUDP(const uint32_t& id, message<T>& msg);
		bool IsRunning();
		bool isClientConnected(const uint32_t& ID)const;
	};

	template <typename T>
	bool server_interface<T>::IsConnected(const Socket_t& socket)const
	{
		bool isConnected = false;

		if (m_socketInfo.find(socket.tcp) != m_socketInfo.end())
		{
			if (socket.tcp != INVALID_SOCKET)
			{
				isConnected = true;
			}
		}

		return isConnected;
	}

	template <typename T>
	bool server_interface<T>::isClientConnected(const uint32_t& ID)const
	{
		bool isConnected = false;

		if (m_connections.find(ID) != m_connections.end())
		{
			if (m_connections.at(ID) != INVALID_SOCKET)
			{
				isConnected = true;
			}
		}

		return isConnected;
	}

	template <typename T>
	void server_interface<T>::AlertThread()
	{
		//LOG_INFO("Thread id: %lu alerted!", GetCurrentThreadId());
	}

	template <typename T>
	void server_interface<T>::PrimeReadValidation(SOCKET_INFORMATION<T>* SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&SI->handshakeIn;
		context->DataBuf.len = sizeof(uint64_t);
		context->state = NetState::READ_VALIDATION;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->socket.tcp, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (GetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("Error sending puzzle with error: %ld", GetLastError());
			}
		}
	}

	template <typename T>
	void server_interface<T>::PrimeReadHeader(SOCKET_INFORMATION<T>* SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&SI->msgTempIn.header;
		context->DataBuf.len = sizeof(msg_header<T>);
		context->state = NetState::READ_HEADER;
		DWORD BytesReceived = 0;
		DWORD flags = 0;

		if (WSARecv(SI->socket.tcp, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			DWORD error = GetLastError();
			if (error != WSA_IO_PENDING)
			{
				if (error == WSAECONNRESET)
				{
					if (IsConnected(SI->socket))
					{
						DisconnectClient(SI);
					}
				}
				delete context;
				LOG_ERROR("WSARecv header with error: %ld", error);
			}
		}
	}

	template <typename T>
	void server_interface<T>::PrimeReadPacket(SOCKET_INFORMATION<T>* SI)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		char buffer[10] = {};
		context->DataBuf.buf = buffer;
		context->DataBuf.len = static_cast<ULONG>(sizeof(buffer));
		context->state = NetState::READ_PACKET;
		DWORD bytes = 0;
		DWORD flags = 0;
		SI->socket.len = sizeof(SI->socket.remote);
		if (WSARecvFrom(m_udpSocket, &context->DataBuf, 1, &bytes, &flags, (sockaddr*)&SI->socket.remote, &SI->socket.len, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			DWORD error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				LOG_ERROR("RecvFrom: %d", error);
				delete context;
			}
		}
	}

	template <typename T>
	void server_interface<T>::PrimeReadPayload(SOCKET_INFORMATION<T>* SI)
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

		if (WSARecv(SI->socket.tcp, &context->DataBuf, 1, &BytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			DWORD error = GetLastError();
			if (error != WSA_IO_PENDING)
			{
				if (error == WSAECONNRESET)
				{
					if (IsConnected(SI->socket))
					{
						DisconnectClient(SI);
					}
				}
				delete context;
				LOG_ERROR("WSARecv payload with error: %ld", error);
			}
		}
	}

	template <typename T>
	void server_interface<T>::ReadPayload(SOCKET_INFORMATION<T>*& SI, PER_IO_DATA* context)
	{
		this->m_qMessagesIn.push_back(SI->msgTempIn);
		SI->msgTempIn.payload.clear();
		this->PrimeReadHeader(SI);
	}

	template <typename T>
	void server_interface<T>::ReadHeader(SOCKET_INFORMATION<T>*& SI, PER_IO_DATA* context)
	{
		if (SI->msgTempIn.header.size > 0)
		{
			if (SI->msgTempIn.header.size > 9999999)
			{
				LOG_ERROR("Message corrupted, skipping over!");
				ZeroMemory(&SI->msgTempIn.header, sizeof(msg_header<T>));
				this->PrimeReadHeader(SI);
			}
			else
			{
				this->PrimeReadPayload(SI);
			}
		}
		else
		{
			this->m_qMessagesIn.push_back(SI->msgTempIn);
			ZeroMemory(&SI->msgTempIn.header, sizeof(msg_header<T>));
			this->PrimeReadHeader(SI);
		}
	}

	template <typename T>
	void server_interface<T>::WritePayload()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)m_qMessagesOut.front().msg.payload.data();
		context->DataBuf.len = ULONG(m_qMessagesOut.front().msg.payload.size());
		context->state = NetState::WRITE_PAYLOAD;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(m_qMessagesOut.front().remote, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			DWORD error = GetLastError();
			if (error != WSA_IO_PENDING)
			{
				if (error == WSAECONNRESET)
				{
					closesocket(m_qMessagesOut.front().remote);
				}
				delete context;
				if (error != WSAENOTSOCK && error != WSAECONNRESET)
				{
					LOG_ERROR("WSASend on socket: %lld message with error: %ld", m_qMessagesOut.front().remote, error);
				}
			}
		}
	}

	template <typename T>
	void server_interface<T>::WriteHeader()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&m_qMessagesOut.front().msg.header;
		context->DataBuf.len = ULONG(sizeof(msg_header<T>));
		context->state = NetState::WRITE_HEADER;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(m_qMessagesOut.front().remote, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			DWORD error = GetLastError();
			if (error != WSA_IO_PENDING)
			{
				if (error == WSAECONNRESET)
				{
					closesocket(m_qMessagesOut.front().remote);
				}
				delete context;
				if (error != WSAENOTSOCK && error != WSAECONNRESET)
				{
					LOG_ERROR("WSASend on socket: %lld message with error: %ld", m_qMessagesOut.front().remote, error);
				}
			}
		}
	}

	template <typename T>
	void server_interface<T>::WritePacket()
	{
		owned_message<T> msg = m_qPrioMessagesOut.front();
		if (m_socketInfo.find(msg.remote) == m_socketInfo.end())
		{
			return;
		}

		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)msg.msg.payload.data();
		context->DataBuf.len = ULONG(msg.msg.payload.size());
		context->state = NetState::WRITE_PACKET;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		SOCKET_INFORMATION<T>* SI = m_socketInfo.at(msg.remote);

		if (WSASendTo(m_udpSocket, &context->DataBuf, 1, &BytesSent, flags, reinterpret_cast<sockaddr*>(&SI->socket.remote), SI->socket.len, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			DWORD error = GetLastError();
			if (error != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASendTo: %d", error);
				delete context;
			}
		}

	}

	template <typename T>
	void server_interface<T>::ReadValidation(SOCKET_INFORMATION<T>*& SI, PER_IO_DATA* context)
	{
		memcpy(&SI->handshakeIn, context->DataBuf.buf, sizeof(uint64_t));

		if (SI->handshakeIn == SI->handshakeResult)
		{
			EnterCriticalSection(&lock);
			this->OnClientValidated(SI->socket.tcp);
			LeaveCriticalSection(&lock);
			this->PrimeReadHeader(SI);
		}
		else
		{
			EnterCriticalSection(&lock);
			// Legal because unordered_map will return 0 if nothing was erased
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
		HANDLE accEvent = param;
		while (m_isRunning)
		{
			WSANETWORKEVENTS netEvents;
			DWORD success = 0;

			// Will put thread to sleep unless there is I/O to process or a new connection has been made
			if ((success = WSAWaitForMultipleEvents(1, &accEvent, FALSE, WSA_INFINITE, TRUE)) == WSA_WAIT_FAILED)
			{
				LOG_ERROR("WSAWaitForEvents failed with code: %d", WSAGetLastError());
				continue;
			}
			if (success == WSA_WAIT_TIMEOUT)
			{
				LOG_NETWORK("Timed out!");
				continue;
			}

			WSAEnumNetworkEvents(m_listening, accEvent, &netEvents);

			if (netEvents.lNetworkEvents & FD_ACCEPT && (netEvents.iErrorCode[FD_ACCEPT_BIT] == 0))
			{
				SOCKET clientSocket = WaitForConnection();

				CreateSocketInformation(clientSocket);
			}
		}
		return 0;
	}

	template<typename T>
	void server_interface<T>::DisconnectClient(SOCKET_INFORMATION<T>* SI)
	{
		EnterCriticalSection(&lock);
		EnterCriticalSection(&udpLock);
		if (SI != NULL)
		{
			SOCKET socket = SI->socket.tcp;
			auto it = m_connections.begin();
			while (it != m_connections.end())
			{
				if (it->second == socket)
				{
					m_connections.erase(it);
					break;
				}
				it++;
			}
			if (m_socketInfo.find(socket) != m_socketInfo.end())
			{
				m_socketInfo.erase(socket);
			}
			SI->socket.close();
			delete SI;
			SI = nullptr;

			this->OnClientDisconnect(socket);
		}
		LeaveCriticalSection(&lock);
		LeaveCriticalSection(&udpLock);
	}

	template <typename T>
	bool server_interface<T>::IsRunning()
	{
		return m_isRunning;
	}

	template <typename T>
	bool server_interface<T>::CreateSocketInformation(const SOCKET& s)
	{
		struct sockaddr_in c = {};
		socklen_t cLen = sizeof(c);
		getpeername(s, (struct sockaddr*)&c, &cLen);
		char ipAsString[IPV6_ADDRSTRLEN] = {};
		inet_ntop(c.sin_family, &c.sin_addr, ipAsString, sizeof(ipAsString));
		uint16_t port = GetPort((struct sockaddr*)&c);

		SOCKET_INFORMATION<T>* SI = new SOCKET_INFORMATION<T>;
		SI->socket.tcp = s;
		SI->handshakeIn = 0;
		SI->handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
		SI->handshakeResult = scrambleData(SI->handshakeOut);

		OnClientConnect(ipAsString, port);

		HANDLE sockHandle = CreateIoCompletionPort((HANDLE)s, m_CompletionPort, (ULONG_PTR)SI, 0);

		if (sockHandle == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());
			delete SI;

			return false;
		}

		m_socketInfo[SI->socket.tcp] = SI;

		// Send a puzzle to the client and refuse connection if client fails
		this->WriteValidation(SI->socket.tcp, SI->handshakeOut);
		this->PrimeReadPacket(SI);

		return true;
	}

	template <typename T>
	void server_interface<T>::SendToClientUDP(const uint32_t& id, message<T>& msg)
	{
		if (isClientConnected(id))
		{
			owned_message<T>* remoteMsg = new owned_message<T>;
			remoteMsg->msg = msg;
			remoteMsg->remote = m_connections.at(id);
			if (!PostQueuedCompletionStatus((HANDLE)m_udpHandle, 2, (ULONG_PTR)remoteMsg, NULL))
			{
				LOG_ERROR("PostQueuedCompletionStatus: %d", GetLastError());
			}
		}
		else
		{
			LOG_WARNING("SendToClientUDP: Client not connected!");
		}
	}

	template <typename T>
	void server_interface<T>::SendToClient(const uint32_t& id, message<T>& msg)
	{
		if (isClientConnected(id))
		{
			owned_message<T>* remoteMsg = new owned_message<T>;
			remoteMsg->msg = msg;
			remoteMsg->remote = m_connections.at(id);
			if (!PostQueuedCompletionStatus(m_CompletionPort, 1, (ULONG_PTR)remoteMsg, NULL))
			{
				LOG_ERROR("PostQueuedCompletionStatus: %d", GetLastError());
			}
		}
		else
		{
			LOG_WARNING("SendToClient: Client not connected!");
		}
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
		std::string data = "\nFull socket information:\n";

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
	SOCKET server_interface<T>::CreateSocket(const uint16_t& port, SockType&& type)
	{
		// Get a linked network structure based on provided hints
		struct addrinfo hints, * servinfo, * p;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_flags = AI_PASSIVE;

		if (type == SockType::TCP)
		{
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
		}
		else
		{
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;
		}

		int8_t rv = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			LOG_ERROR("Addrinfo: %d", WSAGetLastError());
			return INVALID_SOCKET;
		}

		SOCKET socket;

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			EnterCriticalSection(&lock);
			std::string socketData = PrintSocketData(p);
			LOG_NETWORK(socketData.c_str());
			LeaveCriticalSection(&lock);

			socket = WSASocket(p->ai_family, p->ai_socktype, p->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

			if (socket == INVALID_SOCKET)
			{
				continue;
			}

			int enable = 1;

			// Options to disable Nagle's algorithm (can queue up multiple packets instead of sending 1 by 1)
			if (type == SockType::TCP)
			{
				if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(int)) != 0)
				{
					LOG_ERROR("setsockopt: %d", WSAGetLastError());
					return false;
				}
			}

			// SO_REUSEADDR will let the server to reuse the port its bound on even if it have not closed 
			// by the the operating system yet. Also it allows UDP/TCP being bound to the same port working
			if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable)) != 0)
			{
				LOG_ERROR("setsockopt: %d", WSAGetLastError());
				return false;
			}

			if (bind(socket, p->ai_addr, static_cast<int>(p->ai_addrlen)) != 0)
			{
				LOG_ERROR("Bind failed with error: %d", WSAGetLastError());
				closesocket(socket);
				socket = INVALID_SOCKET;
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

		return socket;
	}

	template <typename T>
	bool server_interface<T>::Start(const uint16_t& port)
	{
		InitializeCriticalSection(&lock);
		InitializeCriticalSection(&udpLock);
		InitWinsock();
		SYSTEM_INFO SystemInfo;
		LOG_NETWORK("Creating the TCP Socket...");
		if ((m_listening = CreateSocket(port, SockType::TCP)) == INVALID_SOCKET)
		{
			return false;
		}

		LOG_NETWORK("Creating the UDP Socket...");
		if ((m_udpSocket = CreateSocket(port, SockType::UDP)) == INVALID_SOCKET)
		{
			return false;
		}

		// Setup an I/O completion port for TCP
		if ((m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());
			return false;
		}

		// Setup an I/O completion port for UDP
		if ((m_udpHandle = CreateIoCompletionPort((HANDLE)m_udpSocket, NULL, m_udpSocket, 0)) == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());
			return false;
		}

		if (!this->Listen(SOMAXCONN))
		{
			return false;
		}

		// Create an event for the accepting socket
		HANDLE accEvent = WSACreateEvent();

		if (accEvent == WSA_INVALID_EVENT)
		{
			LOG_ERROR("Failed to create event %d", WSAGetLastError());
			return false;
		}
		WSAEventSelect(m_listening, accEvent, FD_ACCEPT);

		m_acceptThread = std::thread(&server_interface<T>::ProcessIncomingConnections, this, accEvent);

		// Determine how many processors are on the system
		GetSystemInfo(&SystemInfo);
		m_nrOfThreads = (int)SystemInfo.dwNumberOfProcessors;
		if (m_nrOfThreads >= 6)
		{
			m_nrOfThreads = 6;
		}
		m_workerThreads = new std::thread[m_nrOfThreads];

		m_isRunning = true;
		// Create worker threads based on the number of processors available on the
		// system. Create two worker threads for each processor
		for (size_t i = 0; i < m_nrOfThreads; i++)
		{
			if (i < (m_nrOfThreads / 2))
			{
				m_workerThreads[i] = std::thread(&server_interface<T>::ProcessUDPIO, this);
			}
			else
			{
				m_workerThreads[i] = std::thread(&server_interface<T>::ProcessTCPIO, this);
			}
		}

		return true;
	}

	template <typename T>
	void server_interface<T>::Stop()
	{
		EnterCriticalSection(&lock);
		EnterCriticalSection(&udpLock);
		m_isRunning = false;
		LOG_INFO("Shutting down server!");
		if (m_listening != INVALID_SOCKET)
		{
			for (auto con : m_connections)
			{
				if (CancelIoEx((HANDLE)con.second, NULL))
				{
					LOG_INFO("Cancelled all IO for client: %lld Checking error code: %d", con.second, GetLastError());
				}
				else
				{
					LOG_INFO("Cancel I/O failed: %d", GetLastError());
				}
			}
			m_socketInfo.clear();
			LeaveCriticalSection(&lock);
			LeaveCriticalSection(&udpLock);
			for (int i = 0; i < m_nrOfThreads; i++)
			{
				QueueUserAPC((PAPCFUNC)server_interface<T>::AlertThread, (HANDLE)m_workerThreads[i].native_handle(), NULL);
				m_workerThreads[i].join();
			}
			QueueUserAPC((PAPCFUNC)server_interface<T>::AlertThread, (HANDLE)m_acceptThread.native_handle(), NULL);
			m_acceptThread.join();
		}
	}

	template <typename T>
	DWORD server_interface<T>::ProcessUDPIO()
	{
		SOCKET_INFORMATION<T>* SI = nullptr;
		PER_IO_DATA* context;
		const DWORD CAP = 50;
		OVERLAPPED_ENTRY Entries[CAP];
		ULONG EntriesRemoved = 0;

		while (m_isRunning)
		{
			memset(Entries, 0, sizeof(Entries));
			if (!GetQueuedCompletionStatusEx(m_udpHandle, Entries, CAP, &EntriesRemoved, WSA_INFINITE, TRUE))
			{
				DWORD LastError = GetLastError();
				if (LastError != ERROR_EXE_MARKED_INVALID)
				{
					LOG_ERROR("%d", LastError);
				}
			}

			for (int i = 0; i < (int)EntriesRemoved; i++)
			{
				if (Entries[i].lpCompletionKey == NULL)
				{
					continue;
				}
				// UDP job to send to a client
				if (Entries[i].dwNumberOfBytesTransferred == 2)
				{
					EnterCriticalSection(&udpLock);
					owned_message<T>* s = (owned_message<T>*)Entries[i].lpCompletionKey;
					if (m_socketInfo.find(s->remote) != m_socketInfo.end())
					{
						bool write_in_progress = !m_qPrioMessagesOut.empty();
						m_qPrioMessagesOut.push_back(std::move(*s));

						if (!write_in_progress)
						{
							this->WritePacket();
						}
					}

					delete s;
					LeaveCriticalSection(&udpLock);
					continue;
				}

				context = (PER_IO_DATA*)Entries[i].lpOverlapped;
				if (context != NULL)
				{
					if (HasOverlappedIoCompleted(Entries[i].lpOverlapped))
					{
						switch (context->state)
						{
						case NetState::WRITE_PACKET:
						{
							EnterCriticalSection(&udpLock);
							m_qPrioMessagesOut.pop_front();

							if (!m_qPrioMessagesOut.empty())
							{
								this->WritePacket();
							}
							LeaveCriticalSection(&udpLock);
							break;
						}
						case NetState::READ_PACKET:
						{
							LOG_INFO("Registered user to receive UDP packets");
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

	template <typename T>
	DWORD server_interface<T>::ProcessTCPIO()
	{
		DWORD BytesTransferred = 0;
		SOCKET_INFORMATION<T>* SI = nullptr;
		PER_IO_DATA* context;
		const DWORD CAP = 50;
		OVERLAPPED_ENTRY Entries[CAP];
		ULONG EntriesRemoved = 0;

		while (m_isRunning)
		{
			SOCKET_INFORMATION<T>* SI = nullptr;
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
				if (Entries[i].lpCompletionKey == NULL)
				{
					continue;
				}
				// TCP job to send to a client
				if (Entries[i].dwNumberOfBytesTransferred == 1)
				{
					EnterCriticalSection(&lock);
					owned_message<T>* s = (owned_message<T>*)Entries[i].lpCompletionKey;
					bool write_in_progress = !m_qMessagesOut.empty();
					m_qMessagesOut.push_back(std::move(*s));

					if (!write_in_progress)
					{
						this->WriteHeader();
					}
					delete s;
					LeaveCriticalSection(&lock);
					continue;
				}

				SI = (SOCKET_INFORMATION<T>*)Entries[i].lpCompletionKey;
				context = (PER_IO_DATA*)Entries[i].lpOverlapped;
				if (SI == NULL)
				{
					continue;
				}
				if (Entries[i].dwNumberOfBytesTransferred == 0)
				{
					this->DisconnectClient(SI);
					if (context != NULL)
					{
						delete context;
					}
					continue;
				}
				if (Entries[i].lpOverlapped != NULL)
				{
					if (HasOverlappedIoCompleted(Entries[i].lpOverlapped))
					{
						// If an I/O was completed but we received no data means a client must've disconnected
						// Basically a memcpy so we have data in correct structure
						// I/O has completed, process it
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
						case NetState::WRITE_HEADER:
						{
							EnterCriticalSection(&lock);
							if (m_qMessagesOut.front().msg.payload.size() > 0)
							{
								this->WritePayload();
							}
							else
							{
								m_qMessagesOut.pop_front();

								if (!m_qMessagesOut.empty())
								{
									this->WriteHeader();
								}
							}
							LeaveCriticalSection(&lock);
							break;
						}
						case NetState::WRITE_PAYLOAD:
						{
							EnterCriticalSection(&lock);
							m_qMessagesOut.pop_front();

							if (!m_qMessagesOut.empty())
							{
								this->WriteHeader();
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