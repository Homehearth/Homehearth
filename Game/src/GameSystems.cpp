#include "GameSystems.h"
void GameSystems::UserInputSystem(Scene& scene)
{
	

//if key code W | A | S | D
	{
		auto group = scene.GetRegistry().group<comp::Transform, comp::Velocity>();
		
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
