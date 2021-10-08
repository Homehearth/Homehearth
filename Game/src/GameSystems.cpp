#include "GameSystems.h"

using namespace network;
void GameSystems::UserInputSystem(Scene& scene, Client& client)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::Player>([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
		{
			velocity.vel.z = -InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
			velocity.vel.x = -InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
		});
}

//System check if mouse ray intersects any of the box collider components in scene
void GameSystems::MRayIntersectBoxSystem(Scene& scene)
{
	float t = 0;
	scene.ForEachComponent<comp::BoxCollider, comp::Transform>([&](comp::BoxCollider& boxCollider, comp::Transform& transform)
	{
		//Collided with mouse TODO make it do someting?
		if(Intersect::RayIntersectBox(InputSystem::Get().GetMouseRay(), boxCollider, t))
		{
			LOG_INFO("Mouse position %f, %f, %f", InputSystem::Get().GetMouseRay().rayPos.x, InputSystem::Get().GetMouseRay().rayPos.y, InputSystem::Get().GetMouseRay().rayPos.z);
			LOG_INFO("Transform position %f, %f, %f", transform.position.x, transform.position.y, transform.position.z);
			//transform.scale = sm::Vector3(1.5f, 1.5f, 1.5f);
		}
		else
			transform.scale = sm::Vector3(1.f, 1.f, 1.f);
		
	});
}

void GameSystems::MaterialTestSystem(Scene& scene)
{
	scene.ForEachComponent<comp::Renderable>([&](comp::Renderable& renderable)
		{
			if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::I, KeyState::RELEASED))
				renderable.model->ChangeMaterial("Chest.mtl");
			else if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::O, KeyState::RELEASED))
				renderable.model->ChangeMaterial("Barrel.mtl");
		});
}


