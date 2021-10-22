#include "EnginePCH.h"
#include "Systems.h"

void Systems::MovementSystem(HeadlessScene& scene, float dt)
{
	//Transform
	scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt](comp::Transform& transform, comp::Velocity& velocity)
	{
		transform.position += velocity.vel * dt;
	});
}

void Systems::MovementColliderSystem(HeadlessScene& scene, float dt)
{
	//BoundingOrientedBox
	scene.ForEachComponent<comp::Transform, comp::BoundingOrientedBox>([&, dt](comp::Transform& transform, comp::BoundingOrientedBox& obb)
	{
		obb.Center = transform.position;
	});

	//BoundingSphere
	scene.ForEachComponent<comp::Transform, comp::BoundingSphere>([&, dt](comp::Transform& transform, comp::BoundingSphere& sphere)
	{
		sphere.Center = transform.position;
	});
}

void Systems::LightSystem(Scene& scene, float dt)
{
	//If you update the lightData update the info to the GPU
	scene.ForEachComponent<comp::Light>([&](comp::Light light)
	{
		scene.GetLights()->EditLight(light.lightData, light.index);
	});

	//If an Entity has both a Light and Transform component use Transform for position
	scene.ForEachComponent<comp::Transform, comp::Light>([&, dt](comp::Transform& transform, comp::Light& light)
	{
		light.lightData.position = sm::Vector4(transform.position.x, transform.position.y, transform.position.z, 1.f);
		scene.GetLights()->EditLight(light.lightData, light.index);
	});

}
void Systems::AISystem(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::NPC>([&](Entity entity, comp::NPC& npc)
	{
		comp::Transform* transform = entity.GetComponent<comp::Transform>();
		//transform->position += sm::Vector3(0.01f, 0.f, 1.f);
	});
}
