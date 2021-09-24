#include "EnginePCH.h"
#include "Systems.h"

void MovementSystem(entt::registry reg, Scene& scene, float dt)
{
	const auto view = reg.view<comp::Transform, comp::Velocity>();

	view.each([&, dt](comp::Transform& transform, comp::Velocity& velocity)
		{
			
			transform.position += velocity.vel * dt;
		
			//reset velocity?	
			velocity.vel = sm::Vector3{ 0.0f, 0.0f, 0.0f};
		});
}
