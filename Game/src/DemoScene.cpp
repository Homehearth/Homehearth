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

	auto box = scene.GetRegistry().create();
	auto& transform2 = scene.GetRegistry().emplace<comp::Transform>(box);
	transform2.position.z = -17.0f;
	auto& renderable2 = scene.GetRegistry().emplace<comp::Renderable>(box);
	renderable2.mesh = ResourceManager::Get().GetResource<RMesh>("Cube.fbx");

	//InputSystem::Get().SetCamera(scene.m_currentCamera.get());

	scene.m_gameCamera.SetFollowVelocity(&velocity);
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
				GameSystems::MRayIntersectBoxSystem(scene);
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
			scene.m_oldDebugCameraPosition = scene.m_currentCamera.get()->GetPosition();
			*scene.m_currentCamera = scene.m_gameCamera;
			scene.m_currentCamera.get()->SetPosition(scene.m_oldGameCameraPosition);

			LOG_INFO("Game Camera selected");
		}
		else if (scene.m_currentCamera->GetCameraType() == CAMERATYPE::PLAY)
		{
			scene.m_oldGameCameraPosition = scene.m_currentCamera.get()->GetPosition();
			*scene.m_currentCamera = scene.m_debugCamera;
			scene.m_currentCamera.get()->SetPosition(scene.m_oldDebugCameraPosition);

			LOG_INFO("Debugg Camera selected");
		}
	}
#endif // DEBUG
}