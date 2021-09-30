#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

#include "Components.h"

Game::Game()
	:Engine()
{
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

bool Game::OnStartup()
{
	LOG_INFO("TEST");

	Scene& demo = Engine::GetScene("Demo");
	// Scene logic
	setupDemoScene(demo);

	//Set as current scene
	Engine::SetScene(demo);

	m_client.Connect("127.0.0.1", 4950);

	return true;
}

void Game::Start()
{
	Engine::Startup();
}

