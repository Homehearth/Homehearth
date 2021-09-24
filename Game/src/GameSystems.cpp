#include "GameSystems.h"
void GameSystems::UserInputSystem(entt::registry reg, Scene& scene)
{
	//InputSystem::Get();

//if key code W | A | S | D
	{
		auto group = reg.group<comp::Transform, comp::Velocity>();
		
		group.each([&](comp::Transform&, comp::Velocity& velocity)
			{
				//if keycode W
				{
					velocity.vel.z = 1.0f;
				}
				//if keycode S
				{
					velocity.vel.z = -1.0f;
				}
				//if keycode D
				{
					velocity.vel.x = 1.0f;
				}
				//if keycode A
				{
					velocity.vel.x = -1.0f;
				}
			});
	}
}
