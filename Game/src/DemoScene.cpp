#include "DemoScene.h"

DemoScene::DemoScene(Engine& engine, Client& client, uint32_t* playerID, uint32_t* gameID)
	: SceneBuilder(engine)
	, m_gameID(gameID)
	, m_playerID(playerID)
	, m_client(client)
{
	m_engine = &engine;
	//Initialize player entity
	m_player = CreatePlayerEntity();
	SetUpCamera();

	light_t L;
	L.type = TypeLight::DIRECTIONAL;
	L.direction = sm::Vector4(1.f, -1.f, 0.f, 0.f);
	L.color = sm::Vector4(10.f);
	L.position = sm::Vector4(0.f, 0.f, -10.f, 1.f);
	L.range = 75.f;
	L.enabled = 1;

	m_scene.GetLights()->Add(L);

	L.color = sm::Vector4(300.f);
	L.type = TypeLight::POINT;

	m_scene.GetLights()->Add(L);


	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			if(CollisionSystem::Get().IsColliding(m_chest, m_player))
			{
				LOG_INFO("Player is Colliding with box...");
			}
			//System responding to user input
			GameSystems::MRayIntersectBoxSystem(scene);

			int ver = InputSystem::Get().GetAxis(Axis::VERTICAL);
			int hor = InputSystem::Get().GetAxis(Axis::HORIZONTAL);

			if (scene.m_currentCamera.get()->GetCameraType() == CAMERATYPE::PLAY)
			{
				m_player.GetComponent<comp::Velocity>()->vel.z = ver * m_player.GetComponent<comp::Player>()->runSpeed;
				m_player.GetComponent<comp::Velocity>()->vel.x = hor * m_player.GetComponent<comp::Player>()->runSpeed;

				// Updates the position based on input from player
				Systems::MovementSystem(scene, e.dt);
				//System responding to user input
				GameSystems::UserInputSystem(scene, client);

				
				//m_player.GetComponent<comp::Velocity>()->vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * m_player.GetComponent<comp::Player>()->runSpeed;
				//m_player.GetComponent<comp::Velocity>()->vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * m_player.GetComponent<comp::Player>()->runSpeed;
			}

			if (m_client.IsConnected() && *m_gameID != UINT32_MAX)
			{
				// send updated player position
				network::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_Update;
				comp::Transform t = *m_player.GetComponent<comp::Transform>();
				msg << t << *m_playerID << *m_gameID;
				m_client.Send(msg);
			}
		
			GameSystems::MRayIntersectBoxSystem(scene);

			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(scene);
			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(scene);
		});

	//On collision event add entitys as pair in the collision system
	m_scene.on<ESceneCollision>([&](const ESceneCollision& e, Scene& scene)
		{
			if(e.obj1 != e.obj2)
				CollisionSystem::Get().AddPair(e.obj1, e.obj2);
		});
}

void DemoScene::SetUpCamera()
{
	m_gameCamera.Initialize(sm::Vector3(0, 0, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::PLAY);
	m_scene.m_currentCamera = std::make_unique<Camera>(m_gameCamera);

#ifdef _DEBUG
	m_debugCamera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	//m_scene.m_currentCamera = std::make_unique<Camera>(m_debugCamera);

#endif // DEBUG
	InputSystem::Get().SetCamera(m_scene.m_currentCamera.get());
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
	playerEntity.AddComponent<comp::Transform>();
	comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
	playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };
	comp::Velocity* playeerVelocity = playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;
	
	renderable->model = ResourceManager::Get().GetResource<RModel>("cube.obj");

	m_gameCamera.SetFollowVelocity(playeerVelocity);


	this->m_chest = m_scene.CreateEntity();
	comp::Transform* transform = m_chest.AddComponent<comp::Transform>();
	transform->position.z = 5;

	comp::Velocity* chestVelocity = m_chest.AddComponent<comp::Velocity>();
	comp::BoundingSphere* obb = m_chest.AddComponent<comp::BoundingSphere>();
	obb->Center = transform->position;
	obb->Radius = 2.0f;
	comp::Renderable* renderable2 = m_chest.AddComponent<comp::Renderable>();

	renderable2->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");

	return playerEntity;
}