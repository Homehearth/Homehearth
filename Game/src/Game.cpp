#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

#include "Components.h"

Game::Game() 
{
	// Engine setup
	m_engine.Startup();


	// DEMO Scene
	// Create or get scene
	Scene& demo = m_engine.GetScene("Demo");
	// Scene logic
	setupDemoScene(m_engine, demo);
	
	//Set as current scene
	m_engine.SetScene(demo);

	
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

}	//monster dies here

void Game::Run() 
{
	m_engine.Run();
}
