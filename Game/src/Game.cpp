#include "Game.h"

#include "RMesh.h"

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
	entt::entity entity = demo.CreateEntity();
	comp::Transform& transform = demo.AddComponent<comp::Transform>(entity);
	demo.AddComponent<comp::Mesh>(entity).mesh = ResourceManager::GetResource<RMesh>("Monster.fbx");

	ResourceManager::GetResource<RMesh>("Cube.fbx");


}

void Game::Run() 
{
	m_engine.Run();
}
