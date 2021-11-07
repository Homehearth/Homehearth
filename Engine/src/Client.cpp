#include "EnginePCH.h"
#include "Client.h"

void Client::OnDisconnect()
{
	this->onDisconnectHandler();
}

Client::Client(std::function<void(message<GameMsg>&)> handler, std::function<void()> OnDisconnectFunc)
	:client_interface<GameMsg>(handler), onDisconnectHandler(OnDisconnectFunc)
{
	m_latency = 0;
}

Client::~Client()
{
}

void Client::Update(size_t nMaxMessage)
{
	//TCP
	size_t nMessageCount = 0;
	while (nMessageCount < nMaxMessage && !m_qMessagesIn.empty())
	{
		auto msg = m_qMessagesIn.pop_front();

		this->OnMessageReceived(msg);

		nMessageCount++;
	}

	//UDP
	nMessageCount = 0;
	while (nMessageCount < nMaxMessage && !m_qPrioMessagesIn.empty())
	{
		auto msg = m_qPrioMessagesIn.pop_front();
		msg.header.id = GameMsg::Game_Snapshot;

		this->OnMessageReceived(msg);
		
		nMessageCount++;
	}
}

void Client::OnValidation()
{
	EnterCriticalSection(&lock);
	LOG_INFO("Your connection has been validated!");
	LeaveCriticalSection(&lock);
}

void Client::OnMessageReceived(message<GameMsg>& msg)
{
	EnterCriticalSection(&lock);
	this->messageReceivedHandler(msg);
	LeaveCriticalSection(&lock);
}

void Client::OnConnect()
{
	EnterCriticalSection(&lock);
	LOG_INFO("Connected to the server!");
	LeaveCriticalSection(&lock);
}