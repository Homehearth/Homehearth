#include "EnginePCH.h"
#include "Systems.h"

void Systems::MovementSystem(Scene& scene, float dt)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity>([&, dt](Entity e, comp::Transform& transform, comp::Velocity& velocity)
		{
			transform.position += velocity.vel * dt;
		});
}
