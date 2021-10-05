#include "GameSystems.h"
void GameSystems::UserInputSystem(Scene& scene)
{
		auto view = scene.GetRegistry().view<comp::Transform, comp::Velocity, comp::Player>();
		view.each([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
			{
				velocity.vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
				velocity.vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
			});
}

//System check if mouse ray intersects any of the box collider components in scene
void GameSystems::MRayIntersectBoxSystem(Scene& scene)
{
	float t = 0;
	auto view = scene.GetRegistry().view<comp::BoxCollider>();
	view.each([&](comp::BoxCollider& boxCollider)
	{
		//Collided with mouse TODO make it do someting?
		bool hasCollided = Intersect::RayIntersectBox(InputSystem::Get().GetMouseRay(), boxCollider, t);
	});
}


