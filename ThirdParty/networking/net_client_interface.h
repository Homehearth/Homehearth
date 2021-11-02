#pragma once
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_common.h"
#define PRINT_NETWORK_DEBUG

namespace network
{
	/*
			The interface will initialize winsock on its own and all socket programming is abstracted away.
			From this interface you can use simple functions as connect or disconnect or to see wether or not
			you are connected to the server.
	*/
	template <typename T>
	class client_interface
	{
	private:
		SOCKET m_socket;
		HANDLE m_CompletionPort;
		struct sockaddr_in m_endpoint;
		socklen_t m_endpointLen;
		uint64_t m_handshakeIn;
		uint64_t m_handshakeOut;
		message<T> tempMsgIn;
		std::thread* m_workerThread;
		tsQueue<message<T>> m_qMessagesOut;

	protected:
		CRITICAL_SECTION lock;
		std::function<void(message<T>&)> messageReceivedHandler;

	public:
		tsQueue<message<T>> m_qMessagesIn;

	private:
		// Initialize winsock
		void InitWinsock();
		std::string PrintSocketData(struct addrinfo* p);
		SOCKET CreateSocket(const char* ip, uint16_t& port);
		DWORD WINAPI ProcessIO();

		/*
				Because the nature of async I/O the internal functions will PRIME the socket
				to read or write data. It might be a pending call, hence I make a callback
				to a completion routine once the recv/send call is completed. At that point
				the data is accessible.
		*/
		void PrimeReadValidation();
		void PrimeReadHeader();
		void PrimeReadPayload();
		void ReadHeader(PER_IO_DATA* context);
		void ReadPayload(PER_IO_DATA* context);
		void ReadValidation(PER_IO_DATA* context);
		void WriteHeader();
		void WritePayload();
		void WriteValidation();

		static VOID CALLBACK AlertThread()
		{
			LOG_INFO("Thread was alerted!");
		}

	protected:
		// Functions that runs once when an event happens
		virtual void OnMessageReceived(message<T>& msg) = 0;
		virtual void OnConnect() = 0;
		virtual void OnDisconnect() = 0;
		virtual void OnValidation() = 0;

	public:

	public:
		client_interface(std::function<void(message<GameMsg>&)> handler)
			:messageReceivedHandler(handler)
		{
			m_socket = INVALID_SOCKET;
			m_endpointLen = sizeof(m_endpoint);
			ZeroMemory(&m_endpoint, m_endpointLen);
			m_handshakeIn = 0;
			m_handshakeOut = 0;

			InitWinsock();
			this->m_workerThread = nullptr;
		}

		client_interface()
		{
			m_socket = INVALID_SOCKET;
			m_endpointLen = sizeof(m_endpoint);
			ZeroMemory(&m_endpoint, m_endpointLen);
			m_handshakeIn = 0;
			m_handshakeOut = 0;

			InitWinsock();
			this->m_workerThread = nullptr;
		}

		client_interface<T>& operator=(const client_interface<T>& other) = delete;
		client_interface(const client_interface<T>& other) = delete;

		virtual ~client_interface()
		{
			WSACleanup();
			if (m_workerThread)
			{
				m_workerThread->join();
				delete m_workerThread;
			}
		}
	public:
		// Given IP and port establish a connection to the server
		bool Connect(const char* ip, uint16_t port);
		// Disconnect from the server (THREAD SAFE)
		void Disconnect();
		// Check to see if client is connected to a server (THREAD SAFE)
		bool IsConnected();
		// Sends a message to the server
		void Send(message<T>& msg);
	};

	template <typename T>
	void client_interface<T>::PrimeReadPayload()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)tempMsgIn.payload.data();
		context->DataBuf.len = static_cast<ULONG>(tempMsgIn.payload.size());
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
	void client_interface<T>::ReadValidation(PER_IO_DATA* context)
	{
		if (IsConnected())
		{
			WriteValidation();
		}
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
	void client_interface<T>::ReadHeader(PER_IO_DATA* context)
	{ 
		if (tempMsgIn.header.size > sizeof(msg_header<T>))
		{
			if (tempMsgIn.header.size > 9999999)
			{
				LOG_ERROR("Message corrupted, ignoring read!");
				this->PrimeReadHeader();
			}
			else
			{
				tempMsgIn.payload.resize(tempMsgIn.header.size - sizeof(msg_header<T>));
				this->PrimeReadPayload();
			}
		}
		else
		{
			this->m_qMessagesIn.push_back(tempMsgIn);
			this->PrimeReadHeader();
		}
	}

	template <typename T>
	void client_interface<T>::WriteHeader()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)&m_qMessagesOut.front().header;
		context->DataBuf.len = ULONG(sizeof(msg_header<T>));
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
	void client_interface<T>::WritePayload()
	{
		PER_IO_DATA* context = new PER_IO_DATA;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
		context->DataBuf.buf = (CHAR*)m_qMessagesOut.front().payload.data();
		context->DataBuf.len = ULONG(m_qMessagesOut.front().payload.size());
		context->state = NetState::WRITE_PAYLOAD;
		DWORD BytesSent = 0;
		DWORD flags = 0;

		if (WSASend(m_socket, &context->DataBuf, 1, &BytesSent, flags, &context->Overlapped, NULL) == SOCKET_ERROR)
		{
			DWORD error = GetLastError();
			if (error != WSA_IO_PENDING)
			{
				if (error == WSAECONNRESET)
				{
					closesocket(m_socket);
				}
				delete context;
				if (error != WSAENOTSOCK)
				{
					LOG_ERROR("WSASend on socket: %lld message with error: %ld", m_socket, error);
				}
			}
		}
	}

	template <typename T>
	void client_interface<T>::ReadPayload(PER_IO_DATA* context)
	{
		this->m_qMessagesIn.push_back(tempMsgIn);
		tempMsgIn.payload.clear();
		this->PrimeReadHeader();
	}

	template <typename T>
	SOCKET client_interface<T>::CreateSocket(const char* ip, uint16_t& port)
	{
		SOCKET sock = INVALID_SOCKET;
		// Get a linked network structure based on provided hints
		struct addrinfo hints, * servinfo, * p;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		int8_t rv = getaddrinfo(ip, std::to_string(port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			LOG_ERROR("Addrinfo: %ld", WSAGetLastError());
			return INVALID_SOCKET;
		}

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			EnterCriticalSection(&lock);
			LOG_NETWORK("%s", PrintSocketData(p).c_str());
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
		if (IsConnected())
		{
			message<T>* mess = new message<T>;
			*mess = msg;
			if (!PostQueuedCompletionStatus(m_CompletionPort, 1, (ULONG_PTR)mess, NULL))
			{
				LOG_ERROR("PostQueuedCompletionStatus: %d", GetLastError());
			}
		}
		else
		{
			LOG_WARNING("Send: No connection!");
		}
	}

	template<typename T>
	inline bool client_interface<T>::Connect(const char* ip, uint16_t port)
	{
		if (IsConnected())
		{
			return false;
		}

		m_socket = CreateSocket(ip, port);

		if (m_socket == INVALID_SOCKET)
		{
			return false;
		}

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
		if (m_workerThread)
		{
			m_workerThread->join();
			delete m_workerThread;
		}
		// Primes the async handle with a receive call to process incoming messages
		this->PrimeReadValidation();

		m_workerThread = new std::thread(&client_interface<T>::ProcessIO, this);

		return true;
	}

	template<typename T>
	inline void client_interface<T>::Disconnect()
	{
		if (IsConnected())
		{
			EnterCriticalSection(&lock);
			if (closesocket(m_socket) != 0)
			{
				LOG_ERROR("Failed to close socket!");
			}
			m_socket = INVALID_SOCKET;

			this->OnDisconnect();

			QueueUserAPC((PAPCFUNC)AlertThread, m_workerThread->native_handle(), NULL);

			CloseHandle(m_CompletionPort);
			LeaveCriticalSection(&lock);
		}
	}

	template<typename T>
	inline bool client_interface<T>::IsConnected()
	{
		EnterCriticalSection(&lock);
		bool isConnected = false;
		if (m_socket != INVALID_SOCKET)
		{
			isConnected = true;
		}
		LeaveCriticalSection(&lock);
		return isConnected;
	}

	template <typename T>
	DWORD client_interface<T>::ProcessIO()
	{
		DWORD BytesTransferred = 0;
		DWORD flags = 0;
		PER_IO_DATA* context = nullptr;
		const DWORD CAP = 50;
		OVERLAPPED_ENTRY Entries[CAP];
		ULONG EntriesRemoved = 0;

		while (IsConnected())
		{
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

				if (Entries[i].dwNumberOfBytesTransferred == 1)
				{
					message<T>* s = (message<T>*)Entries[i].lpCompletionKey;
					bool bWritingMessage = !m_qMessagesOut.empty();
					m_qMessagesOut.push_back(*s);

					if (!bWritingMessage)
					{
						this->WriteHeader();
					}
					delete s;

					continue;
				}
				context = (PER_IO_DATA*)Entries[i].lpOverlapped;

				if (Entries[i].dwNumberOfBytesTransferred == 0)
				{
					if (context != NULL)
					{
						delete context;
					}
					this->Disconnect();
					continue;
				}

				if (Entries[i].lpOverlapped != NULL)
				{
					// I/O has completed, process it
					if (HasOverlappedIoCompleted(Entries[i].lpOverlapped))
					{
						switch (context->state)
						{
						case NetState::READ_VALIDATION:
						{
							this->OnConnect();
							this->ReadValidation(context);
							break;
						}
						case NetState::WRITE_VALIDATION:
						{
							this->PrimeReadHeader();
							break;
						}
						case NetState::READ_HEADER:
						{
							this->ReadHeader(context);
							break;
						}
						case NetState::READ_PAYLOAD:
						{
							this->ReadPayload(context);
							break;
						}
						case NetState::WRITE_HEADER:
						{
							if (m_qMessagesOut.front().payload.size() > 0)
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
							break;
						}
						case NetState::WRITE_PAYLOAD:
						{
							m_qMessagesOut.pop_front();

							if (!m_qMessagesOut.empty())
							{
								this->WriteHeader();
							}

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
		}
		return 0;
	}
}
