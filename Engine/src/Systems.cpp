#include "EnginePCH.h"
#include "Systems.h"

void Systems::MovementSystem(Scene& scene, float dt)
{

	const auto view = scene.GetRegistry().view<comp::Transform, comp::Velocity>();

	view.each([&, dt](comp::Transform& transform, comp::Velocity& velocity)
		{

			transform.position += velocity.vel * dt;
		});
}
