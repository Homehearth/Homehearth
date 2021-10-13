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
	m_scene.on<ESceneUpdate>([&](const ESceneUpdate& e, HeadlessScene& scene)
		{
			//System responding to user input
			GameSystems::MRayIntersectBoxSystem(m_scene);

			int ver = InputSystem::Get().GetAxis(Axis::VERTICAL);
			int hor = InputSystem::Get().GetAxis(Axis::HORIZONTAL);

			if (m_scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
			{
				Systems::MovementSystem(m_scene, e.dt);
			}
			m_scene.GetCurrentCamera()->Update(e.dt);

			GameSystems::MRayIntersectBoxSystem(m_scene);

			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(m_scene);
			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(m_scene);
			this->CheckIfSwappedCamera();
			LOG_INFO("%u", m_scene.GetRegistry()->size());
		});

	//On collision event add entities as pair in the collision system
	m_scene.on<ESceneCollision>([&](const ESceneCollision& e, HeadlessScene& scene)
		{
			LOG_INFO("Collision detected!");
			CollisionSystem::Get().AddPair(e.obj1, e.obj2);
		});
}

void DemoScene::SetUpCamera()
{
	m_debugCamera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	m_scene.SetCurrentCamera(&m_debugCamera);

	InputSystem::Get().SetCamera(m_scene.GetCurrentCamera());
}

void DemoScene::CheckIfSwappedCamera()
{
	CAMERATYPE test = m_scene.GetCurrentCamera()->GetCameraType();

#ifdef _DEBUG
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
	{
		if (m_scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::DEBUG)
		{
			m_scene.SetCurrentCamera(&m_gameCamera);
			LOG_INFO("Game Camera selected");
		}
		else if (m_scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
		{
			m_scene.SetCurrentCamera(&m_debugCamera);
			LOG_INFO("Debugg Camera selected");
		}
	}
#endif // DEBUG
}

void DemoScene::InitializeGameCam()
{
	m_gameCamera.Initialize(sm::Vector3(0, 0, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::PLAY);
	m_scene.SetCurrentCamera(&m_gameCamera);
}

Entity DemoScene::CreatePlayerEntity(uint32_t playerID)
{
	Entity playerEntity = m_scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>();
	comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
	playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };
	comp::Velocity* playerVelocity = playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;
	playerEntity.AddComponent<comp::Network>()->key = playerID;

	renderable->model = ResourceManager::Get().GetResource<RModel>("cube.obj");

	return playerEntity;
}