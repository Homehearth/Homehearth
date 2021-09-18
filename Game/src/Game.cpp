#include "Game.h"

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
}

void Game::Run() 
{
	m_engine.Run();
}
