#include "GameSystems.h"

using namespace network;
void GameSystems::UserInputSystem(Scene& scene, Client<network::GameMsg>& client)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::Player>([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
		{
			velocity.vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
			velocity.vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
		});
}
