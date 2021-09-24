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

void CreateTest(Scene& scene)
{
	entt::entity entity = scene.GetRegistry().create();
	comp::Renderable& rend = scene.GetRegistry().emplace<comp::Renderable>(entity);
	rend.mesh = ResourceManager::GetResource<RMesh>("Cube.fbx");
}

void setupDemoScene(Engine& engine, Scene& scene) 
{

	// System to update triangles
	scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			auto view = scene.GetRegistry().view<Triangle, Velocity>();
			view.each([&](Triangle& triangle, Velocity& velocity)
				{
					sm::Vector2 nextPos = triangle.pos + velocity.vel * e.dt * velocity.mag;
					
					if (nextPos.x > engine.GetWindow()->GetWidth() - triangle.size ||
						nextPos.x < 0)
					{
						velocity.vel.x = -velocity.vel.x;

						TriangleCollisionEvent e = { &triangle, &velocity };
						scene.publish<TriangleCollisionEvent>(e);
					}
					if (nextPos.y > engine.GetWindow()->GetHeight() - triangle.size ||
						nextPos.y < 0)
					{
						velocity.vel.y = -velocity.vel.y;
						TriangleCollisionEvent e = { &triangle, &velocity };
						scene.publish<TriangleCollisionEvent>(e);
					}
					triangle.pos = nextPos;
				});
		});

	// System to render Triangle components
	scene.on<ESceneRender>([&](const ESceneRender&, Scene& scene)
		{
			auto view = scene.GetRegistry().view<Triangle>();
			view.each([](Triangle& triangle)
				{
					D2D1Core::DrawF(triangle.pos.x, triangle.pos.y, triangle.size, triangle.size, Shapes::TRIANGLE_FILLED);
				});
		});
	scene.on<TriangleCollisionEvent>([](const TriangleCollisionEvent& e, Scene& scene)
		{
			// split
			float size = e.triangle->size * 0.7f;
			if (size < 5) return;
			e.triangle->size = size;
			
			const sm::Vector2 velSign(std::signbit(e.velocity->vel.x) ? -1 : 1, std::signbit(e.velocity->vel.y) ? -1 : 1 );
			createTriangle(scene, size, e.triangle->pos, velSign);

		});

	// Create test Entity
	const sm::Vector2 pos = { 0.f, 0.f };
	const sm::Vector2 signVel = { 1, 1 };
	createTriangle(scene, 200, pos, signVel);

}
