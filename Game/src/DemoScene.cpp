#include "DemoScene.h"

DemoScene::DemoScene(Engine& engine)
	: SceneBuilder(engine)
{
	m_engine = &engine;
	SetUpCamera();

	Entity chest = m_scene.CreateEntity();
	comp::Transform* transform = chest.AddComponent<comp::Transform>();
	transform->position.z = 5;
	comp::Velocity* chestVelocity = chest.AddComponent<comp::Velocity>();
	comp::BoundingSphere* obb = chest.AddComponent<comp::BoundingSphere>();
	obb->Center = transform->position;
	obb->Radius = 2.0f;
	comp::Renderable* renderable2 = chest.AddComponent<comp::Renderable>();

	renderable2->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");

	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			//System responding to user input
			GameSystems::MRayIntersectBoxSystem(scene);

			int ver = InputSystem::Get().GetAxis(Axis::VERTICAL);
			int hor = InputSystem::Get().GetAxis(Axis::HORIZONTAL);

			if (scene.m_currentCamera.get()->GetCameraType() == CAMERATYPE::PLAY)
			{
				Systems::MovementSystem(scene, e.dt);
				scene.m_currentCamera->Update(e.dt);
			}
		
			GameSystems::MRayIntersectBoxSystem(scene);

			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(scene);
			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(scene);
			CameraUpdate(e.dt);
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
	//m_scene.m_currentCamera = std::make_unique<Camera>(m_gameCamera);

#ifdef _DEBUG
	m_debugCamera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	m_scene.m_currentCamera = std::make_unique<Camera>(m_debugCamera);

#endif // DEBUG
	InputSystem::Get().SetCamera(m_scene.m_currentCamera.get());
}

void DemoScene::CameraUpdate(float deltaTime)
{
	CAMERATYPE test = m_scene.m_currentCamera->GetCameraType();

#ifdef _DEBUG
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
	{
		if (m_scene.m_currentCamera->GetCameraType() == CAMERATYPE::DEBUG)
		{
			m_oldDebugCameraPosition = m_scene.m_currentCamera->GetPosition();
			m_scene.m_currentCamera = m_gameCamera;
			m_scene.m_currentCamera->SetPosition(m_oldGameCameraPosition);

			LOG_INFO("Game Camera selected");
		}
		else if (m_scene.m_currentCamera->GetCameraType() == CAMERATYPE::PLAY)
		{
			m_oldGameCameraPosition = m_scene.m_currentCamera->GetPosition();
			*m_scene.m_currentCamera = m_debugCamera;
			m_scene.m_currentCamera->SetPosition(m_oldDebugCameraPosition);

			LOG_INFO("Debugg Camera selected");
		}
	}
#endif // DEBUG
}

Entity DemoScene::CreatePlayerEntity(uint32_t playerID)
{
	Entity playerEntity = m_scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>();
	comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
	playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };
	comp::Velocity* playeerVelocity = playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;
	playerEntity.AddComponent<comp::Network>()->key = playerID;
	
	renderable->model = ResourceManager::Get().GetResource<RModel>("cube.obj");

	return playerEntity;
}