#pragma once

#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>

#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.Lib")

#define IPV6_ADDRSTRLEN 46
#define BUFFER_SIZE 4096

namespace network
{
	extern void* get_in_addr(const struct sockaddr* sa);
	extern uint16_t GetPort(const struct sockaddr* sa);
	extern uint64_t scrambleData(uint64_t input);
	extern std::string PrintAddressFamily(const struct sockaddr* sa);

	enum class MessageType : uint32_t
	{
		Unknown,
		Disconnected,
		Connected,
		PingServer
	};
}