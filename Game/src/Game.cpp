#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

#include "Components.h"

Game::Game()
	: Engine()
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

bool Game::OnUserUpdate(float deltaTime)
{
	CameraUpdate(Engine::GetScene("Demo"), deltaTime);

	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::P, KeyState::PRESSED))
	{
		m_client.PingServer();
	}

	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::R, KeyState::PRESSED))
	{
		m_client.Connect("127.0.0.1", 4950);
	}

	if (m_client.IsConnected())
	{
		while (!m_client.m_messagesIn.empty())
		{
			auto msg = m_client.m_messagesIn.pop_front();
			switch (msg.header.id)
			{
			case GameMsg::Server_AssignID:
			{
				msg >> this->localPID;

				LOG_INFO("YOUR ID IS: %lu", this->localPID);
				break;
			}
			}
		}
	}

	return true;
}