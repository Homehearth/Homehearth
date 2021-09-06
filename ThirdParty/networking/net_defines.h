#pragma once

#include "net_common.h"

namespace network
{
	void* get_in_addr(const struct sockaddr* sa)
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

	uint16_t GetPort(const struct sockaddr* sa)
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

	std::string PrintAddressFamily(const struct sockaddr* sa)
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

	uint64_t scrambleData(uint64_t input)
	{
		uint64_t output = input ^ 0xDEADBEEFA0FAAAF;
		output = (output & 0x0F0F0F0F0F0) >> 4 | (output & 0x0E0E0E0E0E0E0) << 4;

		return output;
	}
}