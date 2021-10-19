#include "DemoScene.h"

#include "RAnimation.h"	//***temp***

DemoScene::DemoScene(Engine& engine)
	: SceneBuilder(engine)
{
	m_scene.GetRegistry()->on_construct<comp::Light>().connect<&Lights::Add>(m_scene.GetLights());

	// Setup Cameras
	Entity debugCameraEntity = m_scene.CreateEntity();
	debugCameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	debugCameraEntity.AddComponent<comp::Tag<DEBUGCAMERA>>();

	Entity cameraEntity = m_scene.CreateEntity();
	cameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 0, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::PLAY);
	cameraEntity.AddComponent<comp::Tag<CAMERA>>();

	m_directionalLight = CreateLightEntity({ 0.f, 0.f, 0.f, 0.f }, { 1.f, -1.f, 0.f, 0.f }, { 10.f, 10.f, 10.f, 10.f }, 0, TypeLight::DIRECTIONAL, 1);
	m_pointLight = CreateLightEntity({ 0.f, 8.f, -10.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 300.f, 300.f, 300.f, 300.f }, 75.f, TypeLight::POINT, 1);
	

	InputSystem::Get().SetCamera(m_scene.GetCurrentCamera());

	//Construct collider meshes if colliders are added.
	m_scene.GetRegistry()->on_construct<comp::RenderableDebug>().connect<entt::invoke<&comp::RenderableDebug::InitRenderable>>();
	m_scene.GetRegistry()->on_construct<comp::BoundingOrientedBox>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
	m_scene.GetRegistry()->on_construct<comp::BoundingSphere>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
	for (int i = 0; i < 1; i++)
	{
		// Debug Chest
		Entity chest = m_scene.CreateEntity();
		comp::Transform* transform = chest.AddComponent<comp::Transform>();
		transform->position.z = 5.0f * static_cast<float>(i);
		comp::Velocity* chestVelocity = chest.AddComponent<comp::Velocity>();
		comp::BoundingOrientedBox* sphere = chest.AddComponent<comp::BoundingOrientedBox>();
		sphere->Center = transform->position;
		sphere->Extents = sm::Vector3(2.0f);
		comp::Renderable* renderable2 = chest.AddComponent<comp::Renderable>();

		renderable2->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");
	}
	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&, cameraEntity, debugCameraEntity](const ESceneUpdate& e, HeadlessScene& scene)
		{
			//System responding to user input
			//GameSystems::MRayIntersectBoxSystem(m_scene);

			m_scene.GetCurrentCamera()->Update(e.dt);

			//GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(m_scene);
			//GameSystems::CheckCollisions<comp::BoundingSphere, comp::BoundingSphere>(m_scene);
			//Systems::LightSystem(scene, e.dt);
			//this->CheckIfSwappedCamera();
#ifdef _DEBUG
			GameSystems::RenderIsCollidingSystem(m_scene);
		
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

Entity DemoScene::CreateLightEntity(sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled)
{
	Entity lightEntity = m_scene.CreateEntity();

	lightEntity.AddComponent<comp::Light>();
	lightEntity.GetComponent<comp::Light>()->lightData.position = pos;
	lightEntity.GetComponent<comp::Light>()->lightData.direction = dir;
	lightEntity.GetComponent<comp::Light>()->lightData.color = col;
	lightEntity.GetComponent<comp::Light>()->lightData.range = range;
	lightEntity.GetComponent<comp::Light>()->lightData.type = type;
	lightEntity.GetComponent<comp::Light>()->lightData.enabled = enabled;
	
	m_scene.GetLights()->EditLight(lightEntity.GetComponent<comp::Light>()->lightData, lightEntity.GetComponent<comp::Light>()->index);

	return lightEntity;
}
