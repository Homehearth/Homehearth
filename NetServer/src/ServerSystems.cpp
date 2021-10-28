#include "NetServerPCH.h"
#include "ServerSystems.h"

namespace Systems {
	void CharacterMovement(HeadlessScene& scene, float dt)
	{
		scene.ForEachComponent<comp::Player, comp::Attack, comp::Velocity, comp::Transform>([&](comp::Player& p, comp::Attack& a, comp::Velocity& v, comp::Transform& t)
			{
				if (a.isAttacking) // Should only be done one frame
				{

					Ray_t ray = InputSystem::Get().GetMouseRay();
					Plane_t plane;
					plane.normal = sm::Vector3(0, 1, 0);
					plane.point = t.position;

					sm::Vector3 point(0, 0, 0);
					sm::Vector3 targetDir(1, 0, 0);

					if (ray.Intersects(plane, point))
					{
						targetDir = point - t.position;
						targetDir.Normalize(targetDir);
					}
					else {
						LOG_WARNING("Mouse click ray missed walking plane. Should not happen...");
					}

					//v.vel = sm::Vector3::Zero;
					p.state = comp::Player::State::ATTACK;
					p.targetForward = targetDir;
				}

			});
		// turns player with velocity
		scene.ForEachComponent<comp::Player, comp::Velocity>([&](comp::Player& p, comp::Velocity& v)
			{
				if (v.vel.Length() > 0.001f && p.state != comp::Player::State::ATTACK)
				{
					sm::Vector3 vel;
					v.vel.Normalize(vel);
					
					p.state = comp::Player::State::TURN;
					p.targetForward = vel;
				}				
			});

		// if player is turning turns until forward is reached
		scene.ForEachComponent<comp::Player, comp::Transform>([&](comp::Player& p, comp::Transform& t)
			{
				if (p.state == comp::Player::State::TURN || p.state == comp::Player::State::ATTACK)
				{
					float time = dt * p.runSpeed;
					if (ecs::StepRotateTo(t.rotation, p.targetForward, time))
					{
						p.state = comp::Player::State::IDLE;
					}
				}
			});
	}
}
