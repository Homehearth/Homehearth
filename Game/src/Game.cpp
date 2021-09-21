#include "Game.h"

#include "RMesh.h"

#include "Components.h"

Game::Game() 
{
	// Engine setup
	m_engine.Startup();


	// DEMO Scene
	// Create or get scene
	Scene& demo = m_engine.GetScene("Demo");
	// Scene logic
	//setupDemoScene(m_engine, demo);

	auto e = demo.GetRegistry().create();
	auto& transform = demo.GetRegistry().emplace<comp::Transform>(e);
	auto& renderable = demo.GetRegistry().emplace<comp::Renderable>(e);
	renderable.mesh = ResourceManager::GetResource<RMesh>("Monster.fbx");


	//Set as current scene
	m_engine.SetScene(demo);

	//Testing to load in model in the resource manager
	//ResourceManager::GetResource<RMesh>("Cube.fbx");

}

void Game::Run() 
{
	m_engine.Run();
}
