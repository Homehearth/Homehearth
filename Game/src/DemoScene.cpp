#include "DemoScene.h"

DemoScene::DemoScene(HeadlessEngine& engine, Client& client)
	: SceneBuilder(engine)
{
	// Set up Scene

	//Initialize player entity
	Entity player = CreatePlayerEntity();

	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			//System to update velocity
			Systems::MovementSystem(scene, e.dt);
			//System responding to user input
			GameSystems::UserInputSystem(scene, client);
		});
}


Entity DemoScene::CreatePlayerEntity()
{

	Entity playerEntity = m_scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>()->position.z = -17.0f;
	
	playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;

	renderable->mesh = ResourceManager::Get().GetResource<RMesh>("Monster.fbx");
	return playerEntity;
}