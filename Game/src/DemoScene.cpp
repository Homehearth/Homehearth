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

	//Test box vs mouse collision TODO Remove this
	auto boxEntity = scene.GetRegistry().create();
	auto& transform = scene.GetRegistry().emplace<comp::Transform>(boxEntity);
	transform.position = sm::Vector3(0.0f, 0.0f, -5.0f);
	auto& boxCollider = scene.GetRegistry().emplace<comp::BoxCollider>(boxEntity);
	boxCollider.center = sm::Vector3(0.0f, 0.0f, -5.0f);
	boxCollider.norm[0] = sm::Vector3(1.0f, 0.0f, 0.0f);
	boxCollider.norm[1] = sm::Vector3(0.0f, 1.0f, 0.0f);;
	boxCollider.norm[2] = sm::Vector3(0.0f, 0.0f, 1.0f);;
	boxCollider.halfSize[0] = 1.0f;
	boxCollider.halfSize[1] = 1.0f;
	boxCollider.halfSize[2] = 1.0f;
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
