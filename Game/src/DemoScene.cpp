#include "DemoScene.h"
#include <Engine.h>


void createTriangle(Scene& scene, float size, const sm::Vector2& pos, const sm::Vector2& velSign) 
{
	entt::entity entity = scene.GetRegistry().create();
	Triangle& comp = scene.GetRegistry().emplace<Triangle>(entity);
	comp.pos = pos;
	comp.size = size;
	
	Velocity& vel = scene.GetRegistry().emplace<Velocity>(entity);
	vel.vel.x = ((rand() % 100) / 100.f) * velSign.x;
	vel.vel.y = sqrtf(1 - vel.vel.x * vel.vel.x) * velSign.y;
	vel.mag = (rand() % 200) + 100.f;
}

void setupDemoScene(Engine& engine, Scene& scene) 
{
	//Initialize player entity
	auto playerEntity = scene.GetRegistry().create();
	auto& transform = scene.GetRegistry().emplace<comp::Transform>(playerEntity);
	transform.position.z = -17.0f;
	auto& velocity = scene.GetRegistry().emplace<comp::Velocity>(playerEntity);
	auto& renderable = scene.GetRegistry().emplace<comp::Renderable>(playerEntity);
	auto& player = scene.GetRegistry().emplace<comp::Player>(playerEntity);
	player.runSpeed = 10.f;
	renderable.mesh = ResourceManager::GetResource<RMesh>("Monster.fbx");
	
	//System to update velocity
	scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
	{
		Systems::MovementSystem(scene, e.dt);
	});

	//System responding to user input
	scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
	{
		GameSystems::UserInputSystem(scene);
	});

	// Create test Entity
	const sm::Vector2 pos = { 0.f, 0.f };
	const sm::Vector2 signVel = { 1, 1 };
	createTriangle(scene, 200, pos, signVel);

}
