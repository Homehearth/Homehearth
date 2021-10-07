#include "DemoScene.h"

DemoScene::DemoScene(HeadlessEngine& engine, Client& client, uint32_t* playerID, uint32_t* gameID)
	: SceneBuilder(engine)
	, m_gameID(gameID)
	, m_playerID(playerID)
	, m_client(client)
{
	// Set up Scene

	//Initialize player entity
	m_player = CreatePlayerEntity();

	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			//System responding to user input
			GameSystems::MRayIntersectBoxSystem(scene);
			
			//GameSystems::UserInputSystem(scene, client);
			int ver = InputSystem::Get().GetAxis(Axis::VERTICAL);
			int hor = InputSystem::Get().GetAxis(Axis::VERTICAL);
			
			m_player.GetComponent<comp::Velocity>()->vel.z = ver * m_player.GetComponent<comp::Player>()->runSpeed;
			m_player.GetComponent<comp::Velocity>()->vel.x = hor * m_player.GetComponent<comp::Player>()->runSpeed;
		
			//System to update velocity
			Systems::MovementSystem(scene, e.dt);

			if (m_client.IsConnected() && *m_gameID != UINT32_MAX)
			{
				// send updated player position
				network::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_Update;
				comp::Transform t = *m_player.GetComponent<comp::Transform>();
				msg << t << *m_playerID << *m_gameID;
				m_client.Send(msg);
			}
			
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