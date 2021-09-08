#include "EnginePCH.h"
#include "Client.h"
#include "Engine.h"

Client::Client()
{
	if (Connect("188.148.27.231", 4950))
	{
#ifdef _DEBUG
		std::cout << "Connected to server!" << std::endl;
#endif
		if (thread::IsThreadActive())
		{
			T_CJOB(Client, Update);
		}
	}
	else
	{
#ifdef _DEBUG
		std::cout << "The client could not connect to the server!" << std::endl;
#endif
	}
}

void Client::Update()
{
	UpdateClient(Engine::s_engineRunning);
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
