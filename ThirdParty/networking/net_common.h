#pragma once

#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <WS2tcpip.h>

#define IPV6_ADDRSTRLEN 46
#define BUFFER_SIZE 4096

namespace network
{
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

	enum class MessageType : uint32_t
	{
		Unknown,
		Disconnected,
		Connected,
		PingServer
	};
}