#include "EnginePCH.h"
#include "Client.h"
#include "Engine.h"

Client::Client()
{
	if (Connect("127.0.0.1", 4950))
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

Client::~Client()
{
}

void Client::Update()
{
	while (Engine::s_engineRunning)
	{
		using namespace network;

		key[0] = GetAsyncKeyState('1') & 0x8000;
		key[1] = GetAsyncKeyState('2') & 0x8000;
		key[2] = GetAsyncKeyState('3') & 0x8000;

		if (key[0] && !old_key[0])
		{
			message<MessageType> msg = {};
			msg.header.id = MessageType::PingServer;

			Send(msg);
		}

		for (int i = 0; i < 3; i++)
		{
			old_key[i] = key[i];
		}
	}
}
