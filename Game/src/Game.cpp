#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

Game::Game() 
{
	// Engine setup
	m_engine.Startup();


	// DEMO Scene
	// Create or get scene
	Scene& demo = m_engine.GetScene("Demo");
	// Scene logic
	setupDemoScene(m_engine, demo);

	// Set as current scene
	m_engine.SetScene(demo);


	/*
		Testing new resource manager
	*/
	//RMesh* barrel = ResourceManager::Get().GetResource<RMesh>("Barrel.obj");
	//RMesh* chest = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
	//RMesh* monster = ResourceManager::Get().GetResource<RMesh>("Monster.fbx");
	{
		std::shared_ptr<RMesh> chest1 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
		std::shared_ptr<RMesh> chest2 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
		std::shared_ptr<RMesh> chest3 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
		std::shared_ptr<RMesh> chest4 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");


		//std::shared_ptr<RMesh> monster = ResourceManager::Get().GetResource<RMesh>("Monster.fbx");
		//std::shared_ptr<RMesh> chest3 = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
	}

	ResourceManager::Get().FreeResources();


	//RMesh* cube = ResourceManager::Get().GetResource<RMesh>("Cube.fbx");

}

void Game::Run() 
{
	m_engine.Run();
}
