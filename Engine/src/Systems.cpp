#include "EnginePCH.h"
#include "Systems.h"

void Systems::MovementSystem(Scene& scene, float dt)
{
	//Transform
	scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt](comp::Transform& transform, comp::Velocity& velocity)
		{
			transform.position += velocity.vel * dt;
		});

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
