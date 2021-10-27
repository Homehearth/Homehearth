#include "NetServerPCH.h"
#include "ServerSystems.h"

namespace systems {
	void CharacterMovement(HeadlessScene& scene, float dt)
	{
		
		scene.ForEachComponent<comp::Player, comp::Attack, comp::Velocity, comp::Transform>([&](comp::Player& p, comp::Attack& a, comp::Velocity& v, comp::Transform& t)
			{
				if (a.isAttacking)
				{
					p.isTurning = true;
				}
				if (p.isTurning)
				{
					sm::Vector3 vel(1, 0, 0);
					
					v.vel = sm::Vector3::Zero;

					float time = dt * p.runSpeed;
					if (ecs::StepRotateTo(t.rotation, vel, time))
					{
						p.isTurning = false;
					}
				}
			});

		scene.ForEachComponent<comp::Player, comp::Transform, comp::Velocity>([&](comp::Player& p, comp::Transform& t, comp::Velocity& v)
			{
				if (v.vel.Length() > 0.001f)
				{
					sm::Vector3 vel;
					v.vel.Normalize(vel);

					float time = dt * p.runSpeed;
					ecs::StepRotateTo(t.rotation, vel, time);
				}
			});
	}
}
