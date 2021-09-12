#include "EnginePCH.h"
#include "Client.h"
#include "Engine.h"

Client::Client()
{
}

void Client::Update()
{
	UpdateClient(Engine::IsRunning());
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
	case MessageType::Unknown:
		std::cout << "Broadcast received from server: " << msg << std::endl;
		break;
	}
}

void Client::OnConnect()
{
	std::cout << "Connected to server!" << std::endl;
}
