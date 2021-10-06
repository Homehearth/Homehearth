#include "DemoScene.h"

void InitializePlayerEntity(Scene& scene)
{
	auto playerEntity = scene.GetRegistry().create();
	auto& transform = scene.GetRegistry().emplace<comp::Transform>(playerEntity);
	transform.position.z = -17.0f;
	auto& velocity = scene.GetRegistry().emplace<comp::Velocity>(playerEntity);
	auto& renderable = scene.GetRegistry().emplace<comp::Renderable>(playerEntity);
	auto& player = scene.GetRegistry().emplace<comp::Player>(playerEntity);
	player.runSpeed = 10.f;
	renderable.mesh = ResourceManager::Get().GetResource<RMesh>("Monster.fbx");
}

void setupDemoScene(Scene& scene, Client& client)
{
	//Initialize player entity
	InitializePlayerEntity(scene);
	SetUpCamera(scene);

	scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			if (scene.m_currentCamera.get()->GetCameraType() == CAMERATYPE::PLAY)
			{
				//System to update velocity
				Systems::MovementSystem(scene, e.dt);
				//System responding to user input
				GameSystems::UserInputSystem(scene, client);
			}
		});
}

void SetUpCamera(Scene& scene)
{
	scene.m_gameCamera.Initialize(sm::Vector3(0, 0, 1), sm::Vector3(0, 0, 0), sm::Vector3(0, 1, 0), sm::Vector2((float)Engine::GetWindow()->GetWidth(), (float)Engine::GetWindow()->GetHeight()), CAMERATYPE::PLAY);
	//scene.m_currentCamera = std::make_unique<Camera>(scene.m_gameCamera);

#ifdef _DEBUG
	scene.m_debugCamera.Initialize(sm::Vector3(0, 0, 1), sm::Vector3(0, 0, 0), sm::Vector3(0, 1, 0), sm::Vector2((float)Engine::GetWindow()->GetWidth(), (float)Engine::GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	scene.m_currentCamera = std::make_unique<Camera>(scene.m_debugCamera);

#endif // DEBUG
}

void CameraUpdate(Scene& scene, float deltaTime)
{
	scene.m_currentCamera->Update(deltaTime);
	CAMERATYPE test = scene.m_currentCamera->GetCameraType();

#ifdef _DEBUG
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
	{
		if (scene.m_currentCamera->GetCameraType() == CAMERATYPE::DEBUG)
		{
			*scene.m_currentCamera = scene.m_gameCamera;
			LOG_INFO("Game Camera selected");
		}
		else if (scene.m_currentCamera->GetCameraType() == CAMERATYPE::PLAY)
		{
			*scene.m_currentCamera = scene.m_debugCamera;
			LOG_INFO("Debugg Camera selected");
		}
	}
#endif // DEBUG
}