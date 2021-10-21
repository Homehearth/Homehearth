#include "DemoScene.h"


namespace sceneHelp
{
	Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID)
	{
		Entity playerEntity = scene.CreateEntity();
		playerEntity.AddComponent<comp::Transform>();
	
		comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
		playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };

		comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
		playerEntity.AddComponent<comp::Network>()->id = playerID;

		renderable->model = ResourceManager::Get().GetResource<RModel>("cube.obj");
		return playerEntity;
	}

	Entity CreateLightEntity(Scene& scene, sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled)
	{
		Entity lightEntity = scene.CreateEntity();

		lightEntity.AddComponent<comp::Light>();
		lightEntity.GetComponent<comp::Light>()->lightData.position = pos;
		lightEntity.GetComponent<comp::Light>()->lightData.direction = dir;
		lightEntity.GetComponent<comp::Light>()->lightData.color = col;
		lightEntity.GetComponent<comp::Light>()->lightData.range = range;
		lightEntity.GetComponent<comp::Light>()->lightData.type = type;
		lightEntity.GetComponent<comp::Light>()->lightData.enabled = enabled;

		scene.GetLights()->EditLight(lightEntity.GetComponent<comp::Light>()->lightData, lightEntity.GetComponent<comp::Light>()->index);

		return lightEntity;
	}

	void CreateMainMenuScene(Engine& engine)
	{

	}

	void CreateLobbyScene(Engine& engine)
	{

	}

	void CreateGameScene(Engine& engine)
	{
		Scene& gameScene = engine.GetScene("Game");
		
		//Construct collider meshes if colliders are added.
		gameScene.GetRegistry()->on_construct<comp::RenderableDebug>().connect<entt::invoke<&comp::RenderableDebug::InitRenderable>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingOrientedBox>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingSphere>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::Light>().connect<&Lights::Add>(gameScene.GetLights());

		// Setup Cameras
		Entity debugCameraEntity = gameScene.CreateEntity();
		debugCameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
		debugCameraEntity.AddComponent<comp::Tag<TagType::DEBUG_CAMERA>>();

		Entity cameraEntity = gameScene.CreateEntity();
		cameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 2.8f, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::PLAY);
		cameraEntity.AddComponent<comp::Tag<TagType::CAMERA>>();

		gameScene.SetCurrentCameraEntity(cameraEntity);

		for (int i = 0; i < 5; i++)
		{
			// Debug Chest
			Entity chest = gameScene.CreateEntity();
			comp::Transform* transform = chest.AddComponent<comp::Transform>();
			transform->position.z = 5.0f * static_cast<float>(i) + 0.2f;
			comp::Velocity* chestVelocity = chest.AddComponent<comp::Velocity>();
			comp::BoundingOrientedBox* sphere = chest.AddComponent<comp::BoundingOrientedBox>();
			sphere->Center = transform->position;
			sphere->Extents = sm::Vector3(2.0f);
			comp::Renderable* renderable2 = chest.AddComponent<comp::Renderable>();

			renderable2->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");
		}

		CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { 1.f, -1.f, 0.f, 0.f }, { 10.f, 10.f, 10.f, 10.f }, 0, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 0.f, 8.f, -10.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 300.f, 300.f, 300.f, 300.f }, 75.f, TypeLight::POINT, 1);

		Entity demoScene = gameScene.CreateEntity();
		comp::Transform* DemoSceneTransform = demoScene.AddComponent<comp::Transform>();
		comp::Renderable* demoSceneModel = demoScene.AddComponent<comp::Renderable>();
		demoSceneModel->model = ResourceManager::Get().GetResource<RModel>("Demoscene.fbx");


		InputSystem::Get().SetCamera(gameScene.GetCurrentCamera());


		gameScene.on<ESceneUpdate>([cameraEntity, debugCameraEntity](const ESceneUpdate& e, Scene& scene)
			{
				scene.GetCurrentCamera()->Update(e.dt);

				IMGUI(
					ImGui::Begin("Scene");
					ImGui::Text("Game");
					ImGui::End();
				);

				GameSystems::RenderIsCollidingSystem(scene);

#ifdef _DEBUG
				if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
				{
					if (scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::DEBUG)
					{
						scene.SetCurrentCameraEntity(cameraEntity);
						InputSystem::Get().SwitchMouseMode();
						LOG_INFO("Game Camera selected");
					}
					else if (scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
					{
						scene.SetCurrentCameraEntity(debugCameraEntity);
						InputSystem::Get().SwitchMouseMode();
						LOG_INFO("Debug Camera selected");
					}
				}
#endif // DEBUG

			});
	}
}
