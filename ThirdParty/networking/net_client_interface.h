#pragma once
#include "net_common.h"
#include "net_tsqueue.h"

namespace network
{
	struct PER_IO_DATA
	{
		OVERLAPPED Overlapped;
		CHAR buffer[BUFFER_SIZE] = {};
		WSABUF DataBuf;
		SOCKET socket;
	};

	template <typename T>
	class client_interface
	{
	private:
		struct sockaddr_in m_endpoint;
		socklen_t m_endpointLen;
		CRITICAL_SECTION lock;

		SOCKET m_socket;
		WSAEVENT m_event;

	private:
		std::string PrintSocketData(struct addrinfo* p);
		void InitWinsock();
		SOCKET CreateSocket(std::string& ip, uint16_t& port);
		DWORD WINAPI ProcessIO();

	protected:

	public:
		client_interface()
		{
			m_socket = INVALID_SOCKET;

			m_endpointLen = sizeof(m_endpoint);
			ZeroMemory(&m_endpoint, m_endpointLen);

			InitWinsock();
		}

		virtual ~client_interface()
		{
			Disconnect();
			WSACleanup();
		}

	public:
		virtual void OnMessageReceived(const message<T>& msg) = 0;

		virtual void OnConnect() = 0;

		virtual void OnDisconnect() = 0;

		virtual void OnValidation() = 0;

		// Given IP and port establish a connection to the server
		bool Connect(std::string&& ip, uint16_t&& port);
		// Disconnect from the server
		void Disconnect();
		// Check to see if client is connected to a server
		bool IsConnected();
		void Send(const message<T>& msg);
		void ReadHeader();
		static VOID CALLBACK ReadPayload(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
		void WriteHeader(const message<T>& msg);
		void WritePayload();
		static VOID CALLBACK ReadValidation(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
		static void WriteValidation(SOCKET s, uint64_t handshakeIn);
	};

	template <typename T>
	VOID CALLBACK client_interface<T>::ReadValidation(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
	{
		PER_IO_DATA* context = (PER_IO_DATA*)lpOverlapped;
		uint64_t handshakeIn = 0;
		memcpy(&handshakeIn, context->DataBuf.buf, sizeof(uint64_t));

		if (cbTransferred > 0)
		{
			WriteValidation(context->socket, handshakeIn);
		}
		else
		{
			LOG_WARNING("Lost connection to server!");
		}
		delete context;
	}

	template <typename T>
	void client_interface<T>::WriteValidation(SOCKET s, uint64_t handshakeIn)
	{
		uint64_t handshakeOut = scrambleData(handshakeIn);
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.len = sizeof(uint64_t);
		memcpy(context->buffer, &handshakeOut, context->DataBuf.len);
		context->DataBuf.buf = context->buffer;
		DWORD BytesSent = 0;
		DWORD flags = 0;
		context->socket = s;

		if (WSASend(s, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASend failed!");
				delete context;
			}
			else
			{
				LOG_INFO("WSASend is pending!");
			}
		}
		else
		{
			LOG_INFO("WSASend was ok!");
		}
	}

	template <typename T>
	void client_interface<T>::ReadHeader()
	{
		PER_IO_DATA* context = new PER_IO_DATA;

		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = context->buffer;
		context->DataBuf.len = BUFFER_SIZE;
		context->socket = m_socket;
		DWORD flags = 0;
		DWORD ReceivedBytes = 0;

		if (WSARecv(m_socket, &context->DataBuf, 1, &ReceivedBytes, &flags, &context->Overlapped, ReadPayload) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSARecv failed!");
				delete context;
			}
			else
			{
				EnterCriticalSection(&lock);
				LOG_INFO("WSARecv is pending!");
				LeaveCriticalSection(&lock);
			}
		}
		else
		{
			EnterCriticalSection(&lock);
			LOG_INFO("WSARecv was ok!");
			LeaveCriticalSection(&lock);
		}
	}

	template <typename T>
	void client_interface<T>::WriteHeader(const message<T>& msg)
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->socket = m_socket;
		memcpy(context->buffer, &msg.header, sizeof(msg.header));
		context->DataBuf.buf = context->buffer;
		context->DataBuf.len = sizeof(msg.header);
		DWORD BytesSent = 0;
		DWORD flags = 0;
		
		if (WSASend(context->socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LOG_ERROR("WSASend failed!");
				delete context;
			}
			else
			{
				LOG_INFO("WSASend is pending!");
			}
		}
		else
		{
			LOG_INFO("WSASend was ok!");
		}
	}

	template <typename T>
	VOID CALLBACK client_interface<T>::ReadPayload(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
	{
		PER_IO_DATA* context = (PER_IO_DATA*)lpOverlapped;

		message<MessageType> msg = {};

		memcpy(&msg.header, context->DataBuf.buf, sizeof(msg.header));
		switch (msg.header.id)
		{
		case MessageType::Client_Accepted:
		{
			LOG_INFO("YOU ARE VALIDATED!");
		}
		}
		delete context;
	}

	template <typename T>
	void client_interface<T>::WritePayload()
	{

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
			std::cout << PrintSocketData(p) << std::endl;
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
		DWORD index = 0;
		WSANETWORKEVENTS networkEvents = {};

		while (IsConnected())
		{
			// Will put thread to sleep unless there is I/O to process or a new connection has been made
			if ((index = WSAWaitForMultipleEvents(1, &m_event, FALSE, WSA_INFINITE, TRUE)) == WSA_WAIT_FAILED)
			{
				printf("WSAWaitForMultipleEvents() failed %d\n", WSAGetLastError());
				continue;
			}

			if (index == WAIT_IO_COMPLETION)
			{
				// An overlapped request completion routine
				// just completed. Continue servicing more completion routines.

				//continue;
			}

			WSAEnumNetworkEvents(m_socket, m_event, &networkEvents);

			if (networkEvents.lNetworkEvents & FD_CONNECT && (networkEvents.iErrorCode[FD_CONNECT_BIT] == 0))
			{
				// CALLING THREAD MUST ISSUE A RECEIVE TASK TO USE THE COMPLETION ROUTINE AKA CALLBACK FUNCTION
				PER_IO_DATA* context = new PER_IO_DATA;
				ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
				context->DataBuf.buf = context->buffer;
				context->DataBuf.len = BUFFER_SIZE;
				DWORD flags = 0;
				DWORD bytesReceived = 0;
				context->socket = m_socket;

				if (WSARecv(m_socket, &context->DataBuf, 1, &bytesReceived, &flags, &context->Overlapped, ReadValidation) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						LOG_ERROR("WSARecv failed!");
						delete context;
					}
					else
					{
						EnterCriticalSection(&lock);
						LOG_INFO("WSARecv is pending!");
						LeaveCriticalSection(&lock);
					}
				}
				else
				{
					EnterCriticalSection(&lock);
					LOG_INFO("WSARecv was ok!");
					LeaveCriticalSection(&lock);
				}
			}

			if (networkEvents.lNetworkEvents & FD_READ && (networkEvents.iErrorCode[FD_READ_BIT] == 0))
			{
			}
			if (networkEvents.lNetworkEvents & FD_CLOSE && (networkEvents.iErrorCode[FD_CLOSE_BIT] == 0))
			{
				this->Disconnect();
			}
			if (networkEvents.lNetworkEvents & FD_WRITE && (networkEvents.iErrorCode[FD_WRITE_BIT] == 0))
			{
				EnterCriticalSection(&lock);
				LOG_INFO("READING HEADER!");
				LeaveCriticalSection(&lock);
				ReadHeader();
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

		m_event = WSACreateEvent();

		WSAEventSelect(m_socket, m_event, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

		if (WSASetEvent(m_event) == FALSE)
		{
			printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
			return false;
		}

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
#ifdef _DEBUG
			std::cout << "Failed to close socket: " << WSAGetLastError() << std::endl;
#endif
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