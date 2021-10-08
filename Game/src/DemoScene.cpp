#include "DemoScene.h"

DemoScene::DemoScene(Engine& engine, Client& client, uint32_t* playerID, uint32_t* gameID)
	: SceneBuilder(engine)
	, m_gameID(gameID)
	, m_playerID(playerID)
	, m_client(client)
{
	Entity box = m_scene.CreateEntity();
	box.AddComponent<comp::Transform>()->position.z = -5.0f;
	comp::Renderable* boxRender = box.AddComponent<comp::Renderable>();
	boxRender->mesh = ResourceManager::Get().GetResource<RMesh>("Cube.fbx");

	m_engine = &engine;
	//Initialize player entity
	SetUpCamera();
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
			
			if (scene.m_currentCamera.get()->GetCameraType() == CAMERATYPE::PLAY)
			{
				//System to update velocity
				Systems::MovementSystem(scene, e.dt);
				//System responding to user input
				GameSystems::UserInputSystem(scene, client);
				GameSystems::MRayIntersectBoxSystem(scene);

				//m_player.GetComponent<comp::Velocity>()->vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * m_player.GetComponent<comp::Player>()->runSpeed;
				//m_player.GetComponent<comp::Velocity>()->vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * m_player.GetComponent<comp::Player>()->runSpeed;
			}
		});
}

void DemoScene::SetUpCamera()
{
	m_gameCamera.Initialize(sm::Vector3(0, 0, 1), sm::Vector3(0, 0, 0), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::PLAY);
	//scene.m_currentCamera = std::make_unique<Camera>(m_gameCamera);

#ifdef _DEBUG
	m_debugCamera.Initialize(sm::Vector3(0, 0, 1), sm::Vector3(0, 0, 0), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	m_scene.m_currentCamera = std::make_unique<Camera>(m_debugCamera);

#endif // DEBUG
}

void DemoScene::CameraUpdate(float deltaTime)
{
	m_scene.m_currentCamera->Update(deltaTime);
	CAMERATYPE test = m_scene.m_currentCamera->GetCameraType();

#ifdef _DEBUG
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
	{
		if (m_scene.m_currentCamera->GetCameraType() == CAMERATYPE::DEBUG)
		{
			m_oldDebugCameraPosition = m_scene.m_currentCamera.get()->GetPosition();
			*m_scene.m_currentCamera = m_gameCamera;
			m_scene.m_currentCamera.get()->SetPosition(m_oldGameCameraPosition);

			LOG_INFO("Game Camera selected");
		}
		else if (m_scene.m_currentCamera->GetCameraType() == CAMERATYPE::PLAY)
		{
			m_oldGameCameraPosition = m_scene.m_currentCamera.get()->GetPosition();
			*m_scene.m_currentCamera = m_debugCamera;
			m_scene.m_currentCamera.get()->SetPosition(m_oldDebugCameraPosition);

			LOG_INFO("Debugg Camera selected");
		}
	}
#endif // DEBUG
}

Entity DemoScene::CreatePlayerEntity()
{

	Entity playerEntity = m_scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>()->position.z = -17.0f;
	
	comp::Velocity* playeerVelocity =  playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;

	renderable->mesh = ResourceManager::Get().GetResource<RMesh>("Chest.obj");

	m_gameCamera.SetFollowVelocity(playeerVelocity);

	return playerEntity;
}