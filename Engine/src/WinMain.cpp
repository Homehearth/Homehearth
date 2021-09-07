#include <EnginePCH.h>

#include "Engine.h"
#include "InputSystem.h"

#include <chrono>

void setupDemoScene(Engine& engine, Scene& scene);

// The main entry point of the engine.
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MessageBoxA(nullptr, "Debug mode!", "Engine", 0);
#else
	MessageBoxA(nullptr, "Release mode!", "Engine", 0);
#endif


	Engine engine;
	engine.Setup(hInstance);

	// Create or get scene
	Scene& startScene = engine.GetScene("StartScene");
	// Set as current scene
	engine.SetScene(startScene);

	setupDemoScene(engine, startScene);

	engine.Start();

	return 0;
}

void setupDemoScene(Engine& engine, Scene& scene) {


	// ECS DEMO
	// Simple Components
	struct Triangle {
		float pos[2];
		float size[2];
	};
	struct Velocity {
		float vel[2];
		float mag;
	};


	// System to update components
	scene.AddSystem([&](entt::registry& reg, float dt)
		{
			auto view = reg.view<Triangle, Velocity>();
			view.each([&, dt](Triangle& triangle, Velocity& velocity)
				{
					if (triangle.pos[0] > engine.GetWindow()->GetWidth() - triangle.size[0] ||
						triangle.pos[0] < 0)
					{
						velocity.vel[0] = -velocity.vel[0];
					}
					if (triangle.pos[1] > engine.GetWindow()->GetHeight() - triangle.size[1] ||
						triangle.pos[1] < 0)
					{
						velocity.vel[1] = -velocity.vel[1];
					}
					triangle.pos[0] += velocity.vel[0] * dt * velocity.mag;
					triangle.pos[1] += velocity.vel[1] * dt * velocity.mag;

				});
		});

	// System to render Triangle components
	scene.AddRenderSystem([&](entt::registry& reg)
		{
			auto view = reg.view<Triangle>();
			view.each([](Triangle& triangle)
				{
					D2D1Core::DrawF(triangle.pos[0], triangle.pos[1], triangle.size[0], triangle.size[1], Shapes::TRIANGLE_FILLED);
				});
		});

	// Create test Entities
	for (int i = 0; i < 100; i++)
	{
		entt::entity entity = scene.CreateEntity();
		Triangle& comp = scene.AddComponent<Triangle>(entity);
		comp.pos[0] = 0.0f;
		comp.pos[1] = 0.0f;
		comp.size[0] = (rand() % 70) + 30;
		comp.size[1] = (rand() % 70) + 30;

		Velocity& vel = scene.AddComponent<Velocity>(entity);
		vel.vel[0] = (rand() % 100) / 100.f;
		vel.vel[1] = sqrtf(1 - vel.vel[0] * vel.vel[0]);
		vel.mag = 300.f;
	}

}
	
