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

	//Testing to load in model in the resource manager
	ResourceManager::Get().GetResource<RMesh>("Monster.fbx");
	//ResourceManager::Get().GetResource<RMesh>("Cube.fbx");

}

void Game::Run() 
{
	m_engine.Run();
}
