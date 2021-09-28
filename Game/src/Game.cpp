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
	
	//setupDemoScene(m_engine, demo);
	for (int i = 0; i < 800; i++) {
		auto e = demo.GetRegistry().create();
		auto& transform = demo.GetRegistry().emplace<comp::Transform>(e);
		auto& renderable = demo.GetRegistry().emplace<comp::Renderable>(e);
		renderable.mesh = ResourceManager::GetResource<RMesh>("Cube.fbx");
	}

	
	demo.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene) 
		{
		
			int i = 1;
			demo.GetRegistry().view<comp::Transform>().each([&](comp::Transform& t)
				{
	
					sm::Vector3 pos = t.position;
					t.position = sm::Vector3(0, 0, 0);
					pos += sm::Vector3(0, 0, -i) * e.dt;
					t.position = pos;
									
					i++;

				});
			
		});
	

	//Set as current scene
	m_engine.SetScene(demo);

	//Testing to load in model in the resource manager
	//ResourceManager::GetResource<RMesh>("Cube.fbx");

}

void Game::Run() 
{
	m_engine.Run();
}
