#include "DemoScene.h"

DemoScene::DemoScene(HeadlessEngine& engine, Client& client)
	: SceneBuilder(engine)
{
	// Set up Scene

	//Initialize player entity
	m_player = CreatePlayerEntity();

	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			//System to update velocity
			Systems::MovementSystem(scene, e.dt);
			//System responding to user input
			//GameSystems::UserInputSystem(scene, client);
			GameSystems::MRayIntersectBoxSystem(scene);
			m_player.GetComponent<comp::Velocity>()->vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * m_player.GetComponent<comp::Player>()->runSpeed;
			m_player.GetComponent<comp::Velocity>()->vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * m_player.GetComponent<comp::Player>()->runSpeed;

		});
}

Entity DemoScene::CreatePlayerEntity()
{

	Entity playerEntity = m_scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>()->position.z = -17.0f;
	
	playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;

	renderable->mesh = ResourceManager::Get().GetResource<RMesh>("Chest.obj");
	return playerEntity;
}