#pragma once
#include "net_tsqueue.h"
#define PRINT_NETWORK_DEBUG

namespace network
{
	enum class NetState
	{
		NOT_VALIDATED,
		READ_VALIDATION,
		WRITE_VALIDATION,
		READ_HEADER,
		READ_PAYLOAD,
		WRITE_HEADER,
		WRITE_PAYLOAD
	};

	struct PER_IO_DATA
	{
		OVERLAPPED Overlapped;
		WSABUF DataBuf;
		NetState state;
	};

	/*
			The interface will initialize winsock on its own and all socket programming is abstracted away.
			From this interface you can use simple functions as connect or disconnect or to see wether or not
			you are connected to the server.
	*/
	template <typename T>
	class client_interface
	{
	private:
		HANDLE m_CompletionPort;
		struct sockaddr_in m_endpoint;
		socklen_t m_endpointLen;
		message<T> tempMsgIn;
		SOCKET m_socket;
		uint64_t m_handshakeIn;
		uint64_t m_handshakeOut;
		tsQueue<message<T>> m_messagesIn;

	private:
		// Initialize winsock
		void InitWinsock();
		std::string PrintSocketData(struct addrinfo* p);
		SOCKET CreateSocket(std::string& ip, uint16_t& port);
		// Client worker thread that processes all incoming and outgoing data
		DWORD WINAPI ProcessIO();

		/*
				Because the nature of async I/O the internal functions will PRIME the socket
				to read or write data. It might be a pending call, hence I make a callback
				to a completion routine once the recv/send call is completed. At that point
				the data is accessible.
		*/
		void PrimeReadValidation();
		void PrimeReadHeader();
		void PrimeReadPayload(size_t size);
		void ReadHeader(PER_IO_DATA*& context);
		void ReadPayload(PER_IO_DATA*& context);
		void ReadValidation(PER_IO_DATA*& context);
		void WriteHeader(msg_header<T>& header);
		void WritePayload(message<T>& msg);
		void WriteValidation();

	protected:
		CRITICAL_SECTION lock;

	protected:
		// Functions that runs once when an event happens
		virtual void OnMessageReceived(message<T>& msg) = 0;
		virtual void OnConnect() = 0;
		virtual void OnDisconnect() = 0;
		virtual void OnValidation() = 0;

	public:

	public:
		client_interface()
		{
			m_socket = INVALID_SOCKET;
			m_endpointLen = sizeof(m_endpoint);
			ZeroMemory(&m_endpoint, m_endpointLen);
			m_handshakeIn = 0;
			m_handshakeOut = 0;

			InitWinsock();
		}

		client_interface<T>& operator=(const client_interface<T>& other) = delete;
		client_interface(const client_interface<T>& other) = delete;

		virtual ~client_interface()
		{
			WSACleanup();
		}
	public:

		// Given IP and port establish a connection to the server
		bool Connect(std::string&& ip, uint16_t&& port);
		// Disconnect from the server
		void Disconnect();
		// Check to see if client is connected to a server
		bool IsConnected();
		// Sends a message to the server
		void Send(message<T>& msg);
	};

	template <typename T>
	void client_interface<T>::PrimeReadPayload(size_t size)
	{
		message<T> tempMsg = m_messagesIn.front();
		tempMsg.payload.clear();
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		tempMsg.payload.resize(size);
		context->DataBuf.buf = (CHAR*)&tempMsg.payload[0];
		context->DataBuf.len = (ULONG)size;
		DWORD flags = 0;
		DWORD bytesReceived = 0;
		context->state = NetState::READ_PAYLOAD;

		if (WSARecv(m_socket, &context->DataBuf, 1, &bytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSARecv failed with error: %d", WSAGetLastError());
				delete context;
				context = nullptr;
			}
		}
	}

	template <typename T>
	void client_interface<T>::PrimeReadHeader()
	{
		ZeroMemory(&tempMsgIn.header, sizeof(msg_header<T>));
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&tempMsgIn.header;
		context->DataBuf.len = sizeof(msg_header<T>);
		context->state = NetState::READ_HEADER;
		DWORD flags = 0;
		DWORD ReceivedBytes = 0;

		if (WSARecv(m_socket, &context->DataBuf, 1, &ReceivedBytes, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSARecv failed with error: %d", WSAGetLastError());
				delete context;
				context = nullptr;
			}
		}
	}

	template <typename T>
	void client_interface<T>::PrimeReadValidation()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&this->m_handshakeIn;
		context->DataBuf.len = sizeof(uint64_t);
		DWORD flags = 0;
		DWORD bytesReceived = 0;
		context->state = NetState::READ_VALIDATION;

		if (WSARecv(m_socket, &context->DataBuf, 1, &bytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSARecv failed with error: %d", WSAGetLastError());
				delete context;
				context = nullptr;
			}
		}
	}

	template <typename T>
	void client_interface<T>::ReadValidation(PER_IO_DATA*& context)
	{
		WriteValidation();
	}

	template <typename T>
	void client_interface<T>::WriteValidation()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		this->m_handshakeOut = scrambleData(this->m_handshakeIn);
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&this->m_handshakeOut;
		context->DataBuf.len = sizeof(uint64_t);
		DWORD BytesSent = 0;
		DWORD flags = 0;
		context->state = NetState::WRITE_VALIDATION;

		if (WSASend(m_socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASend failed with error: %d", WSAGetLastError());
				delete context;
				context = nullptr;
			}
		}
	}

	template <typename T>
	void client_interface<T>::ReadHeader(PER_IO_DATA*& context)
	{
		m_messagesIn.push_back(tempMsgIn);
		message<T> tempMsg = m_messagesIn.front();
		if (tempMsg.header.size > sizeof(msg_header<T>))
		{
			if (tempMsg.header.size > 300)
			{
				std::cout << "lol" << std::endl;
			}
			this->PrimeReadPayload(tempMsg.header.size - sizeof(msg_header<T>));
		}
		else
		{
			this->OnMessageReceived(tempMsg);
		}
		m_messagesIn.pop_front();
	}

	template <typename T>
	void client_interface<T>::WriteHeader(msg_header<T>& header)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&header;
		context->DataBuf.len = sizeof(msg_header<T>);
		context->state = NetState::WRITE_HEADER;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(m_socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASend failed with error: %d", WSAGetLastError());
				delete context;
				context = nullptr;
			}
		}
	}

	template <typename T>
	void client_interface<T>::ReadPayload(PER_IO_DATA*& context)
	{
		this->OnMessageReceived(tempMsgIn);
		tempMsgIn.payload.clear();
	}

	template <typename T>
	void client_interface<T>::WritePayload(message<T>& msg)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&msg.payload[0];
		context->DataBuf.len = (ULONG)msg.payload.size();
		context->state = NetState::WRITE_PAYLOAD;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(m_socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASend failed! %d", WSAGetLastError());
				delete context;
				context = nullptr;
			}
		}
	}

	template <typename T>
	SOCKET client_interface<T>::CreateSocket(std::string& ip, uint16_t& port)
	{
		SOCKET sock = INVALID_SOCKET;
		// Get a linked network structure based on provided hints
		struct addrinfo hints, * servinfo, * p;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		int8_t rv = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			LOG_ERROR("Addrinfo: %ld", WSAGetLastError());
			return false;
		}

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			EnterCriticalSection(&lock);
			LOG_NETWORK(PrintSocketData(p).c_str());
			LeaveCriticalSection(&lock);
			sock = WSASocket(p->ai_family, p->ai_socktype, p->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

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
	DWORD client_interface<T>::ProcessIO()
	{
		DWORD BytesTransferred = 0;
		DWORD flags = 0;
		BOOL bResult = false;
		//LPOVERLAPPED lpOverlapped;
		PER_IO_DATA* context = nullptr;
		BOOL shouldDisconnect = false;
		const DWORD CAP = 10;
		OVERLAPPED_ENTRY Entries[CAP];
		ULONG EntriesRemoved = 0;

		while (IsConnected())
		{
			if (!GetQueuedCompletionStatusEx(m_CompletionPort, Entries, CAP, &EntriesRemoved, WSA_INFINITE, TRUE))
			{
				DWORD LastError = GetLastError();
				if (LastError != ERROR_EXE_MARKED_INVALID)
				{
					LOG_ERROR("%d", LastError);
				}

				continue;
			}

			if (EntriesRemoved > 1)
			{
				LOG_WARNING("Entries removed: %u", EntriesRemoved);
			}

			for (int i = 0; i < (int)EntriesRemoved; i++)
			{
				// If an I/O was completed but we received no data means a client must've disconnected
				// Basically a memcpy so we have data in correct structure
				if (Entries[i].dwNumberOfBytesTransferred == 0)
				{
					this->Disconnect();
					delete context;
					continue;
				}
				// I/O has completed, process it
				if (HasOverlappedIoCompleted(Entries[i].lpOverlapped))
				{
					context = (PER_IO_DATA*)Entries[i].lpOverlapped;

					switch (context->state)
					{
					case NetState::READ_VALIDATION:
					{
						this->OnConnect();
#ifdef PRINT_NETWORK_DEBUG
						EnterCriticalSection(&lock);
						LOG_NETWORK("Reading validation! Bytes: %ld", Entries[i].dwNumberOfBytesTransferred);
#endif
						this->ReadValidation(context);
						LeaveCriticalSection(&lock);
						break;
					}
					case NetState::WRITE_VALIDATION:
					{
#ifdef PRINT_NETWORK_DEBUG
						EnterCriticalSection(&lock);
						LOG_NETWORK("Writing validation! Bytes: %ld", Entries[i].dwNumberOfBytesTransferred);
#endif
						this->PrimeReadHeader();
						LeaveCriticalSection(&lock);
						break;
					}
					case NetState::READ_HEADER:
					{
#ifdef PRINT_NETWORK_DEBUG
						EnterCriticalSection(&lock);
						LOG_NETWORK("Reading header! Bytes: %ld", Entries[i].dwNumberOfBytesTransferred);
#endif
						this->ReadHeader(context);
						LeaveCriticalSection(&lock);
						break;
					}
					case NetState::READ_PAYLOAD:
					{
#ifdef PRINT_NETWORK_DEBUG
						EnterCriticalSection(&lock);
						LOG_NETWORK("Reading payload! Bytes: %ld", Entries[i].dwNumberOfBytesTransferred);
#endif
						this->ReadPayload(context);
						LeaveCriticalSection(&lock);
						break;
					}
					case NetState::WRITE_HEADER:
					{
#ifdef PRINT_NETWORK_DEBUG
						EnterCriticalSection(&lock);
						LOG_NETWORK("Writing header! Bytes: %ld", Entries[i].dwNumberOfBytesTransferred);
#endif
						this->PrimeReadHeader();
						LeaveCriticalSection(&lock);
						break;
					}
					case NetState::WRITE_PAYLOAD:
					{
#ifdef PRINT_NETWORK_DEBUG
						EnterCriticalSection(&lock);
						LOG_NETWORK("Writing payload! Bytes: %ld", Entries[i].dwNumberOfBytesTransferred);
						LeaveCriticalSection(&lock);
#endif
						break;
					}
					}

					if (context)
					{
						delete context;
					}
				}
			}
		}
		return 0;
	}

	template<typename T>
	void client_interface<T>::InitWinsock()
	{
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

		char ipAsString[IPV6_ADDRSTRLEN] = {};

		inet_ntop(p->ai_family, get_in_addr(p->ai_addr), ipAsString, sizeof(ipAsString));

		data += "Connecting to: ";
		data += ipAsString;
		data += ":";
		data += std::to_string(GetPort(p->ai_addr));

		data += "\n";
		data += "\n";

		return data;
	}

	template<typename T>
	inline void client_interface<T>::Send(message<T>& msg)
	{
		this->WriteHeader(msg.header);

		if (msg.header.size > 0)
		{
			this->WritePayload(msg);
		}
	}

	template<typename T>
	inline bool client_interface<T>::Connect(std::string&& ip, uint16_t&& port)
	{
		m_socket = CreateSocket(ip, port);

		if (connect(m_socket, (struct sockaddr*)&m_endpoint, m_endpointLen) != 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				EnterCriticalSection(&lock);
				LOG_ERROR("Failed to connect to server!");
				LeaveCriticalSection(&lock);
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;

				return false;
			}
		}

		if ((m_CompletionPort = CreateIoCompletionPort((HANDLE)m_socket, NULL, (ULONG_PTR)&m_socket, 0)) == NULL)
		{
			LOG_ERROR("CreateIoCompletionPort() failed with error %d", GetLastError());
			return false;
		}

		// Primes the async handle with a receive call to process incoming messages
		this->PrimeReadValidation();

		std::thread t(&client_interface<T>::ProcessIO, this);
		t.detach();

		return true;
	}

	template<typename T>
	inline void client_interface<T>::Disconnect()
	{
		EnterCriticalSection(&lock);
		if (!IsConnected())
		{
			LeaveCriticalSection(&lock);
			return;
		}
		if (closesocket(m_socket) != 0)
		{
			LOG_ERROR("Failed to close socket!");
		}

		m_socket = INVALID_SOCKET;
		this->OnDisconnect();
		LeaveCriticalSection(&lock);
	}

	template<typename T>
	inline bool client_interface<T>::IsConnected()
	{
		if (m_socket == INVALID_SOCKET)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}