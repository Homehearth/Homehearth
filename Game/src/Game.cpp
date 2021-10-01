#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

#include "Components.h"

Game::Game()
	: Engine()
{
	this->localPID = 0;
	/*
		Resource manager example
	*/
	/*
	std::shared_ptr<RMesh> monster = ResourceManager::Get().GetResource<RMesh>("Monster.fbx");

	{	//Start a scope for show

		std::shared_ptr<RMesh> chest1 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
		std::shared_ptr<RMesh> chest2 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
		std::shared_ptr<RMesh> chest3 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");

	}	//chest1,2,3 dies here

	//Clearing up resources after chest1
	ResourceManager::Get().FreeResources();
	*/

}

Game::~Game()
{
	m_client.Disconnect();
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
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::P, KeyState::PRESSED))
	{
		m_client.PingServer();
	}

	if (m_client.IsConnected())
	{
		while (!m_client.m_messagesIn.empty())
		{
			message<GameMsg> msg = m_client.m_messagesIn.pop_front();
			switch (msg.header.id)
			{
			case GameMsg::Server_AssignID:
			{
				msg >> this->localPID;

				LOG_INFO("YOUR ID IS: %lld", this->localPID);
				break;
			}
			}
		}
	}

	return true;
}