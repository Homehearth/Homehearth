#pragma once

#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <functional>
#include <unordered_map>
#include "net_message.h"

namespace network
{
#define IPV6_ADDRSTRLEN 46
#define BUFFER_SIZE 32768
	
	struct Socket_t
	{
		SOCKET tcp;
		sockaddr_in remote = {};
		socklen_t len = sizeof(remote);

	public:
		void close()
		{
			closesocket(tcp);
		}
	};

	enum class SockType
	{
		UDP,
		TCP
	};

	template <typename T>
	struct SOCKET_INFORMATION
	{
		uint64_t handshakeIn = 0;
		uint64_t handshakeOut = 0;
		uint64_t handshakeResult = 0;
		Socket_t socket = {};
		message<T> msgTempIn = {};
	};

	// What current state are the current connection in
	enum class NetState
	{
		WRITE_VALIDATION,
		READ_VALIDATION,
		READ_HEADER,
		READ_PAYLOAD,
		WRITE_PACKET,
		READ_PACKET,
		WRITE_HEADER,
		WRITE_PAYLOAD
	};

	// Information regarding every input or output
	struct PER_IO_DATA
	{
		OVERLAPPED Overlapped = {};
		WSABUF DataBuf = {};
		NetState state;
	};

	static void* get_in_addr(const struct sockaddr* sa)
	{
		if (sa->sa_family == AF_INET)
		{
			return &(((struct sockaddr_in*)sa)->sin_addr);
		}
		else
		{
			return &(((struct sockaddr_in6*)sa)->sin6_addr);
		}
	}

	static uint16_t GetPort(const struct sockaddr* sa)
	{
		if (sa->sa_family == AF_INET)
		{
			return ntohs(((struct sockaddr_in*)sa)->sin_port);
		}
		else
		{
			return ntohs(((struct sockaddr_in6*)sa)->sin6_port);
		}
	}

	static std::string PrintAddressFamily(const struct sockaddr* sa)
	{
		if (sa->sa_family == AF_INET)
		{
			return "<IP version 4>";
		}
		else
		{
			return "<IP version 6>";
		}
	}

	static uint64_t scrambleData(uint64_t input)
	{
		uint64_t output = input ^ 0xDEADBEEFA0FAAAF;
		output = (output & 0x0F0F0F0F0F0) >> 4 | (output & 0x0E0E0E0E0E0E0) << 4;

		return output;
	}
}