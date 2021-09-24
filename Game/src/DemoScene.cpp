#include "DemoScene.h"
#include "Components.h"


void createTriangle(Scene& scene, float size, const float pos[2], const int velSign[2]) 
{
	entt::entity entity = scene.CreateEntity();
	Triangle& comp = scene.AddComponent<Triangle>(entity);
	comp.pos[0] = pos[0];
	comp.pos[1] = pos[1];
	comp.size[0] = size;
	comp.size[1] = comp.size[0];

	Velocity& vel = scene.AddComponent<Velocity>(entity);
	vel.vel[0] = ((rand() % 100) / 100.f) * velSign[0];
	vel.vel[1] = sqrtf(1 - vel.vel[0] * vel.vel[0]) * velSign[1];
	vel.mag = (rand() % 200) + 100.f;


}

void CreateTest(Scene& scene)
{
	entt::entity object = scene.CreateEntity();
	comp::RenderAble& rend = scene.AddComponent<comp::RenderAble>(object);
	rend.mesh = ResourceManager::GetResource<RMesh>("Monster.fbx");
	rend.renderForm.position = { 0.0f, 0.0f, 0.0f };
	rend.renderForm.rotation = { 0.0f, 0.0f, 0.0f };
	rend.renderForm.scale = { 0.0f, 0.0f, 0.0f };
	rend.texture = ResourceManager::GetResource<RTexture>("monster_albedo.png");

	comp::Transform& transf = scene.AddComponent<comp::Transform>(object);
	transf.position = { (float)(rand() % 100), (float)(rand() % 100), (float)(rand() % 100) };
	transf.rotation = { (float)(rand() % 100), (float)(rand() % 100), (float)(rand() % 100) };
	transf.scale = { 1, 1, 1 };
}

void setupDemoScene(Engine& engine, Scene& scene) 
{

	/*
	// System to update triangles
	scene.AddSystem([&](entt::registry& reg, float dt)
		{
			auto view = reg.view<Triangle, Velocity>();
			view.each([&, dt](Triangle& triangle, Velocity& velocity)
				{
					float nextPos[2] =
					{
						triangle.pos[0] + velocity.vel[0] * dt * velocity.mag,
						triangle.pos[1] + velocity.vel[1] * dt * velocity.mag,
					};
					if (nextPos[0] > engine.GetWindow()->GetWidth() - triangle.size[0] ||
						nextPos[0] < 0)
					{
						velocity.vel[0] = -velocity.vel[0];

						TriangleCollisionEvent e = { &triangle, &velocity };
						scene.publish<TriangleCollisionEvent>(e);
					}
					if (nextPos[1] > engine.GetWindow()->GetHeight() - triangle.size[1] ||
						nextPos[1] < 0)
					{
						velocity.vel[1] = -velocity.vel[1];
						TriangleCollisionEvent e = { &triangle, &velocity };
						scene.publish<TriangleCollisionEvent>(e);
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
	scene.on<TriangleCollisionEvent>([](const TriangleCollisionEvent& e, Scene& scene)
		{
			// split
			float size = e.triangle->size[0] * 0.7f;
			if (size < 5) return;
			e.triangle->size[0] = size;
			e.triangle->size[1] = size;
			const int velSign[2] = { std::signbit(e.velocity->vel[0]) ? -1 : 1, std::signbit(e.velocity->vel[1]) ? -1 : 1 };
			createTriangle(scene, size, e.triangle->pos, velSign);

		});

	// Create test Entity
	const float pos[2] = { 0.f, 0.f };
	const int signVel[2] = { 1, 1 };
	createTriangle(scene, 200, pos, signVel);
	*/

	for (int i = 0; i < 2500; i++)
	{
		CreateTest(scene);
	}

}
