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
	renderable.model = ResourceManager::Get().GetResource<RModel>("Chest.obj");
}

void setupDemoScene(Scene& scene, Client& client)
{
	//Initialize player entity
	InitializePlayerEntity(scene);

	scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			//System to update velocity
			Systems::MovementSystem(scene, e.dt);
			//System responding to user input
			GameSystems::UserInputSystem(scene, client);
			GameSystems::MRayIntersectBoxSystem(scene);
		});
}
