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