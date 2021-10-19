#include "DemoScene.h"

#include "RAnimation.h"	//***temp***

DemoScene::DemoScene(Engine& engine)
	: SceneBuilder(engine)
{
	// Setup Cameras
	Entity debugCameraEntity = m_scene.CreateEntity();
	debugCameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	debugCameraEntity.AddComponent<comp::Tag<DEBUGCAMERA>>();

	Entity cameraEntity = m_scene.CreateEntity();
	cameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 0, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::PLAY);
	cameraEntity.AddComponent<comp::Tag<CAMERA>>();

	InputSystem::Get().SetCamera(m_scene.GetCurrentCamera());

	//Construct collider meshes if colliders are added.
	m_scene.GetRegistry()->on_construct<comp::RenderableDebug>().connect<entt::invoke<&comp::RenderableDebug::InitRenderable>>();
	m_scene.GetRegistry()->on_construct<comp::BoundingOrientedBox>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
	m_scene.GetRegistry()->on_construct<comp::BoundingSphere>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
	
	// Debug Chest
	Entity chest = m_scene.CreateEntity();
	comp::Transform* transform = chest.AddComponent<comp::Transform>();
	transform->position.z = 5;
	comp::Velocity* chestVelocity = chest.AddComponent<comp::Velocity>();
	comp::BoundingSphere* sphere = chest.AddComponent<comp::BoundingSphere>();
	sphere->Center = transform->position;
	sphere->Radius = 2.0f;
	comp::Renderable* renderable2 = chest.AddComponent<comp::Renderable>();

	renderable2->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");


	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&, cameraEntity, debugCameraEntity](const ESceneUpdate& e, HeadlessScene& scene)
		{
			//System responding to user input
			//GameSystems::MRayIntersectBoxSystem(m_scene);

			m_scene.GetCurrentCamera()->Update(e.dt);

			//GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(m_scene);
			//GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(m_scene);
			//this->CheckIfSwappedCamera();
#ifdef _DEBUG
			if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
			{
				if (m_scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::DEBUG)
				{
					m_scene.SetCurrentCamera(&cameraEntity.GetComponent<comp::Camera3D>()->camera);
					InputSystem::Get().SwitchMouseMode();
					LOG_INFO("Game Camera selected");
				}
				else if (m_scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
				{
					m_scene.SetCurrentCamera(&debugCameraEntity.GetComponent<comp::Camera3D>()->camera);
					InputSystem::Get().SwitchMouseMode();
					LOG_INFO("Debug Camera selected");
				}
			}
#endif // DEBUG
		});
	
	//On collision event add entities as pair in the collision system
	m_scene.on<ESceneCollision>([&](const ESceneCollision& e, HeadlessScene& scene)
		{
			LOG_INFO("Collision detected!");
			CollisionSystem::Get().AddPair(e.obj1, e.obj2);
		});
}

Entity DemoScene::CreatePlayerEntity(uint32_t playerID)
{
	Entity playerEntity = m_scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>();
	comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
	playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };
	comp::Velocity* playerVelocity = playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	renderable->model = ResourceManager::Get().GetResource<RModel>("Player_Skeleton.fbx");

	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;
	playerEntity.AddComponent<comp::Network>()->id = playerID;

	RAnimation test;
	test.Create("Player_Idle.fbx");
	std::array<UINT, 3> lastKeys = { 0,0,0 };
	sm::Matrix testmat = test.GetMatrix("QuickRigCharacter_Hips", 0.01, 0.02, lastKeys, true);

	return playerEntity;
}