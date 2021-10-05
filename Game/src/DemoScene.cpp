#include "DemoScene.h"

void InitializePlayerEntity(Scene& scene)
{
	//auto playerEntity = scene.GetRegistry().create();
	//auto& transform = scene.GetRegistry().emplace<comp::Transform>(playerEntity);
	//transform.position.z = -17.0f;
	//auto& velocity = scene.GetRegistry().emplace<comp::Velocity>(playerEntity);
	//auto& renderable = scene.GetRegistry().emplace<comp::Renderable>(playerEntity);
	//auto& player = scene.GetRegistry().emplace<comp::Player>(playerEntity);
	//player.runSpeed = 10.f;
	//renderable.mesh = ResourceManager::Get().GetResource<RMesh>("Monster.fbx");

	//Test box vs mouse collision
	auto boxEntity = scene.GetRegistry().create();
	auto& transform = scene.GetRegistry().emplace<comp::Transform>(boxEntity);
	transform.position = sm::Vector3(0.0f, 0.0f, -5.0f);
	auto& boxCollider = scene.GetRegistry().emplace<comp::SphereCollider>(boxEntity);
	boxCollider.center = sm::Vector3(0.0f, 0.0f, -5.0f);;
	boxCollider.centerOffset = sm::Vector3(0.0f, 0.0f, -5.0f);
	boxCollider.radius = 1.f;
	auto& renderable = scene.GetRegistry().emplace<comp::Renderable>(boxEntity);
	renderable.mesh = ResourceManager::Get().GetResource<RMesh>("Cube.fbx");
	
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
