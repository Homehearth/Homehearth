#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

#include "Components.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1))
	, Engine()
{
	this->localPID = 0;
}

Game::~Game()
{
	if (m_client.IsConnected())
	{
		m_client.Disconnect();
	}
}

bool Game::OnStartup()
{
	m_client.Connect("127.0.0.1", 4950);
	Scene& demo = Engine::GetScene("Demo");
	// Scene logic
	setupDemoScene(demo, m_client);

	//Set as current scene
	SetScene(demo);

	return true;
}

void Game::Start()
{
	Engine::Startup();
}

void Game::CheckIncoming(message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Client_Accepted:
	{
		LOG_INFO("You are validated!");
		break;
	}
	case GameMsg::Server_GetPing:
	{
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		LOG_INFO("Ping: %fs", std::chrono::duration<double>(timeNow - this->timeThen).count());
		break;
	}
	case GameMsg::Server_AssignID:
	{
		msg >> this->localPID;

		LOG_INFO("YOUR ID IS: %lu", this->localPID);
		break;
	}
	case GameMsg::Game_AddPlayer:
	{
		uint32_t remotePlayerID;
		msg >> remotePlayerID;
		LOG_INFO("Player with ID: %ld has joined the game!", remotePlayerID);
		break;
	}
	}
}

void Game::PingServer()
{
	message<GameMsg> msg = {};
	msg.header.id = GameMsg::Server_GetPing;
	msg << this->localPID;

	this->timeThen = std::chrono::system_clock::now();
	m_client.Send(msg);
}

bool Game::OnUserUpdate(float deltaTime)
{
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::P, KeyState::PRESSED))
	{
		this->PingServer();
	}

	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::R, KeyState::PRESSED))
	{
		m_client.Connect("127.0.0.1", 4950);
	}

	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::C, KeyState::PRESSED))
	{
		message<GameMsg> msg; 

		msg.header.id = GameMsg::Client_CreateLobby;
		msg << this->localPID;
		m_client.Send(msg);

		LOG_INFO("Creating game lobby!");
	}
	
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::J, KeyState::PRESSED))
	{
		message<GameMsg> msg;

		msg.header.id = GameMsg::Client_JoinLobby;
		uint32_t lobbyID;
		std::cout << "Type in the lobby ID you wish to join: ";
		std::cin >> lobbyID;
		msg << this->localPID << lobbyID;
		m_client.Send(msg);
	}

	if (m_client.IsConnected())
	{	
		m_client.Update();
	}

	return true;
}