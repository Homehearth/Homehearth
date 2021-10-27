#include "NetServerPCH.h"
#include "ServerSystems.h"

namespace systems {
	void CharacterMovement(HeadlessScene& scene, float dt)
	{
		scene.ForEachComponent<comp::Player, comp::Transform, comp::Velocity>([&](comp::Player& p, comp::Transform& t, comp::Velocity& v)
			{
				if (v.vel.Length() > 0.001f)
				{
					sm::Vector3 vel;
					v.vel.Normalize(vel);
					
					float time = dt * p.runSpeed;

					float targetRotation = atan2(-vel.z, vel.x);
					t.rotation.y = t.rotation.y * (1 - time) + targetRotation * time;


				}
			});
	}
}
