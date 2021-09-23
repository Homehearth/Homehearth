#pragma once
#include "net_tsqueue.h"
//#define PRINT_NETWORK_DEBUG

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
		WSAOVERLAPPED Overlapped;
		CHAR buffer[BUFFER_SIZE] = {};
		WSABUF DataBuf;
		NetState state;
	};

	template <typename T>
	class client_interface
	{
	private:
		struct sockaddr_in m_endpoint;
		socklen_t m_endpointLen;
		CRITICAL_SECTION lock;
		HANDLE m_CompletionPort;
		message<T> tempMsg;
		SOCKET m_socket;

		WSAEVENT m_event;

	private:
		std::string PrintSocketData(struct addrinfo* p);
		void InitWinsock();
		SOCKET CreateSocket(std::string& ip, uint16_t& port);
		DWORD WINAPI ProcessIO();

		/*
				Because the nature of async I/O the internal functions will PRIME the socket
				to read or write data. It might be a pending call, hence I make a callback
				to a completion routine once the recv/send call is completed. At that point
				the data is accessible.
		*/
		void PrimeReadValidation();
		void PrimeReadHeader();
		void PrimeReadPayload(uint32_t size);
		void ReadHeader(PER_IO_DATA*& context);
		void ReadPayload(PER_IO_DATA*& context);
		void ReadValidation(PER_IO_DATA*& context);
		void WriteHeader(message<T>& msg);
		void WritePayload(message<T>& msg);
		void WriteValidation(uint64_t handshakeIn);

	protected:

	public:
		client_interface()
		{
			m_socket = INVALID_SOCKET;

			m_endpointLen = sizeof(m_endpoint);
			ZeroMemory(&m_endpoint, m_endpointLen);
			tempMsg = {};

			InitWinsock();
		}

		client_interface<T>& operator=(const client_interface<T>& other) = delete;
		client_interface(const client_interface<T>& other) = delete;

		virtual ~client_interface()
		{
			Disconnect();
			WSACleanup();
		}
		tsQueue<message<T>> messages;
	public:
		virtual void OnMessageReceived(message<T>& msg) = 0;

		virtual void OnConnect() = 0;

		virtual void OnDisconnect() = 0;

		virtual void OnValidation() = 0;

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
	void client_interface<T>::PrimeReadPayload(uint32_t size)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(WSAOVERLAPPED));
		context->DataBuf.buf = context->buffer;
		context->DataBuf.len = (ULONG)size;
		DWORD flags = 0;
		DWORD bytesReceived = 0;
		context->state = NetState::READ_PAYLOAD;

		if (WSARecv(m_socket, &context->DataBuf, 1, &bytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSARecv failed! %d", WSAGetLastError());
				delete context;
			}
		}
	}

	template <typename T>
	void client_interface<T>::PrimeReadHeader()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(WSAOVERLAPPED));
		context->DataBuf.buf = context->buffer;
		context->DataBuf.len = sizeof(msg_header<T>);
		context->state = NetState::READ_HEADER;
		DWORD flags = 0;
		DWORD ReceivedBytes = 0;

		if (WSARecv(m_socket, &context->DataBuf, 1, &ReceivedBytes, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSARecv failed! %d", WSAGetLastError());
				delete context;
			}
		}
	}

	template <typename T>
	void client_interface<T>::PrimeReadValidation()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(WSAOVERLAPPED));
		context->DataBuf.buf = context->buffer;
		context->DataBuf.len = sizeof(uint64_t);
		DWORD flags = 0;
		DWORD bytesReceived = 0;
		context->state = NetState::READ_VALIDATION;

		if (WSARecv(m_socket, &context->DataBuf, 1, &bytesReceived, &flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				delete context;
				LOG_ERROR("WSARecv failed! %d", WSAGetLastError());
			}
		}
	}

	template <typename T>
	void client_interface<T>::ReadValidation(PER_IO_DATA*& context)
	{
		uint64_t handshakeIn = 0;
		memcpy(&handshakeIn, context->DataBuf.buf, sizeof(uint64_t));

		WriteValidation(handshakeIn);
	}

	template <typename T>
	void client_interface<T>::WriteValidation(uint64_t handshakeIn)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		uint64_t handshakeOut = scrambleData(handshakeIn);
		ZeroMemory(&context->Overlapped, sizeof(WSAOVERLAPPED));
		context->DataBuf.len = sizeof(uint64_t);
		memcpy(context->buffer, &handshakeOut, context->DataBuf.len);
		context->DataBuf.buf = context->buffer;
		DWORD BytesSent = 0;
		DWORD flags = 0;
		context->state = NetState::WRITE_VALIDATION;

		if (WSASend(m_socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASend failed! %d", WSAGetLastError());
				delete context;
			}
		}
	}

	template <typename T>
	void client_interface<T>::ReadHeader(PER_IO_DATA*& context)
	{
		memcpy(&tempMsg.header, context->DataBuf.buf, sizeof(msg_header<T>));

		if (tempMsg.header.size > 0)
		{
			tempMsg.payload.clear();
			this->PrimeReadPayload(tempMsg.header.size - sizeof(msg_header<T>));
		}
		else
		{
			this->OnMessageReceived(tempMsg);
		}
	}

	template <typename T>
	void client_interface<T>::WriteHeader(message<T>& msg)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(WSAOVERLAPPED));
		memcpy(context->buffer, &msg.header, sizeof(msg.header));
		context->DataBuf.buf = context->buffer;
		context->DataBuf.len = sizeof(msg.header);
		context->state = NetState::WRITE_HEADER;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(m_socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASend failed! %d", WSAGetLastError());
				delete context;
			}
		}
	}

	template <typename T>
	void client_interface<T>::ReadPayload(PER_IO_DATA*& context)
	{
		tempMsg.payload.resize(context->DataBuf.len);
		memcpy(&tempMsg.payload[0], context->DataBuf.buf, context->DataBuf.len);

		this->OnMessageReceived(tempMsg);
	}

	template <typename T>
	void client_interface<T>::WritePayload(message<T>& msg)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(WSAOVERLAPPED));
		memcpy(context->buffer, &msg.payload[0], msg.payload.size());
		context->DataBuf.buf = context->buffer;
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
#ifdef _DEBUG
			LOG_NETWORK(PrintSocketData(p).c_str());
#endif
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
		EnterCriticalSection(&lock);
		LOG_INFO("Client created successfully!");
		LeaveCriticalSection(&lock);
		DWORD BytesTransferred = 0;
		DWORD flags = 0;
		BOOL bResult = false;
		LPOVERLAPPED lpOverlapped;
		PER_IO_DATA* context = nullptr;
		BOOL shouldDisconnect = false;
		SOCKET* sock;

		while (IsConnected())
		{
			bResult = GetQueuedCompletionStatus(m_CompletionPort, &BytesTransferred, (PULONG_PTR)&sock, &lpOverlapped, WSA_INFINITE);

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

			if (shouldDisconnect)
			{
				Disconnect();
			}

			context = CONTAINING_RECORD(lpOverlapped, PER_IO_DATA, PER_IO_DATA::Overlapped);

			switch (context->state)
			{
			case NetState::READ_VALIDATION:
			{
				this->OnConnect();
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_NETWORK("Reading validation!");
				LeaveCriticalSection(&lock);
#endif
				this->ReadValidation(context);
				break;
			}
			case NetState::WRITE_VALIDATION:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_NETWORK("Writing validation!");
				LeaveCriticalSection(&lock);
#endif
				this->PrimeReadHeader();
				break;
			}
			case NetState::READ_HEADER:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_NETWORK("Reading header!");
				LeaveCriticalSection(&lock);
#endif
				this->ReadHeader(context);
				break;
			}
			case NetState::READ_PAYLOAD:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_NETWORK("Reading payload!");
				LeaveCriticalSection(&lock);
#endif
				this->ReadPayload(context);
				break;
			}
			case NetState::WRITE_HEADER:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_NETWORK("Writing header!");
				LeaveCriticalSection(&lock);
#endif
				this->PrimeReadHeader();
				break;
			}
			case NetState::WRITE_PAYLOAD:
			{
#ifdef PRINT_NETWORK_DEBUG
				EnterCriticalSection(&lock);
				LOG_NETWORK("Writing payload!");
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
		return 0;
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
		this->WriteHeader(msg);

		if (msg.payload.size() > 0)
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
		else
		{
			LOG_INFO("CompletionPort OK!");
		}

		PrimeReadValidation();

		std::thread t(&client_interface<T>::ProcessIO, this);
		t.detach();

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
			LOG_ERROR("Failed to close socket!");
		}

		m_socket = INVALID_SOCKET;
		this->OnDisconnect();
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