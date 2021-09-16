#include "EnginePCH.h"
#include "Client.h"
#include "Engine.h"

Client::Client()
{
}

void Client::OnDisconnect()
{
	std::cout << "Disconnected from server!" << std::endl;
}

Client::~Client()
{
}

void Client::OnMessageReceived(const network::message<network::MessageType>& msg)
{
	using namespace network;

	switch (msg.header.id)
	{
	case MessageType::Connected:
		std::cout << "Server sent: " << msg << std::endl;
		break;
	}
}

void Client::OnConnect()
{
	std::cout << "Connected to server!" << std::endl;
}
