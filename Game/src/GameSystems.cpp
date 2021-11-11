#include "GameSystems.h"
#include "EnginePCH.h"
#include "Healthbar.h"


//System check if mouse ray intersects any of the box collider components in scene
void GameSystems::MRayIntersectBoxSystem(Scene& scene)
{
	float t = 0;

	scene.ForEachComponent<comp::BoundingOrientedBox, comp::Transform>([&](Entity entity, comp::BoundingOrientedBox& boxCollider, comp::Transform& transform)
		{
			//Collided with mouse TODO make it do someting?
			if (Intersect::RayIntersectBox(InputSystem::Get().GetMouseRay(), boxCollider, t))
			{
				LOG_INFO("Mouseray HIT box detected!");
			}
		});
}

//System to render collider mesh red if collider is colliding with another collider
void GameSystems::RenderIsCollidingSystem(Scene& scene)
{
	scene.ForEachComponent<comp::RenderableDebug>([&](Entity entity, comp::RenderableDebug& renderableDebug)
		{
			//Collided with mouse TODO make it do someting?
			if (CollisionSystem::Get().GetCollisionCounts(entity) > 0)
			{
				renderableDebug.isColliding.hit = 1;
			}
			else
			{
				renderableDebug.isColliding.hit = 0;
			}
		});
}

// Set all the healthbars to players.
void GameSystems::UpdateHealthbar(Scene& scene)
{
	int i = 1;
	scene.ForEachComponent<comp::Health, comp::Player>([&](Entity e, comp::Health& health, const comp::Player& player) {
		// Safety check for now.
		if (i < 5)
		{
			rtd::Healthbar* healthbar = dynamic_cast<rtd::Healthbar*>(scene.GetCollection("player" + std::to_string(i) + "Info")->elements[0].get());
			if (healthbar)
			{
				healthbar->SetHealthVariable(e);
			}
		}
		i++;
		});
}

void GameSystems::UpdatePlayerVisuals(Scene& scene)
{
	int i = 1;
	int j = 4;
	scene.ForEachComponent<comp::NamePlate, comp::Transform>([&](Entity& e, comp::NamePlate& name, comp::Transform& t)
		{
			if (i < 5)
			{
				Collection2D* collection = scene.GetCollection("dynamicPlayer" + std::to_string(i) + "namePlate");
				if (collection)
				{
					rtd::Text* namePlate = dynamic_cast<rtd::Text*>(collection->elements[0].get());
					if (namePlate)
					{
						Camera* cam = scene.GetCurrentCamera();

						if (cam->GetCameraMatrixes())
						{
							// Conversion from World space to NDC space.
							sm::Vector4 oldP = { t.position.x, t.position.y + 25.0f, t.position.z, 1.0f };
							sm::Vector4 newP = dx::XMVector4Transform(oldP, cam->GetCameraMatrixes()->view);
							newP = dx::XMVector4Transform(newP, cam->GetCameraMatrixes()->projection);
							newP.x /= newP.w;
							newP.y /= newP.w;
							newP.z /= newP.w;

							// Conversion from NDC space [-1, 1] to Window space
							float new_x = (((newP.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
							float new_y = D2D1Core::GetWindow()->GetHeight() - (((newP.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

							namePlate->SetPosition(new_x - ((name.namePlate.length() * D2D1Core::GetDefaultFontSize()) * 0.5f), new_y);
							// Show nameplates only if camera is turned to it.
							if (newP.z < 1.f)
								namePlate->SetVisiblity(true);
							else
								namePlate->SetVisiblity(false);

							// Update healthbars position.
							Collection2D* collHealth = scene.GetCollection("player" + std::to_string(i) + "Info");
							if (collHealth)
							{
								rtd::Healthbar* health = dynamic_cast<rtd::Healthbar*>(collHealth->elements[0].get());
								if (health)
								{
									sm::Vector4 oldPp = { t.position.x, t.position.y + 20.0f, t.position.z, 1.0f };
									sm::Vector4 newPp = dx::XMVector4Transform(oldPp, cam->GetCameraMatrixes()->view);
									newPp = dx::XMVector4Transform(newPp, cam->GetCameraMatrixes()->projection);
									newPp.x /= newPp.w;
									newPp.y /= newPp.w;
									newPp.z /= newPp.w;

									// Conversion from NDC space [-1, 1] to Window space
									new_x = (((newPp.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
									new_y = D2D1Core::GetWindow()->GetHeight() - (((newPp.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

									health->SetPosition(new_x - (health->GetOpts().width * 0.5f), new_y);

									// Only visible if camera is turned to it.
									if (newPp.z < 1.f)
										health->SetVisiblity(true);
									else
										health->SetVisiblity(false);
								}
							}
						}
					}
				}
			}
			i++;
			j--;
		});

}


// Will check if a ray hits an object before it hits the player and if it does it will add the object to be rendered transparent
void GameSystems::CheckLOS(Scene& scene, const sm::Vector3& playerPos, const std::vector<dx::BoundingOrientedBox>& mapColliders)
{
	Camera* cam = scene.GetCurrentCamera();

	// Only need to check LOS if we are using the game camera
	if (cam->GetCameraType() == CAMERATYPE::PLAY)
	{
		// Shoot a ray from cameras position to players position
		Ray_t ray;
		ray.origin = cam->GetPosition();
		ray.dir = (playerPos - ray.origin);
		ray.dir.Normalize(ray.dir);

		for (int i = 0; i < mapColliders.size(); i++)
		{
			if (ray.Intersects(mapColliders[i]))
			{
				LOG_INFO("You are now behind an object and we cannot see you ROFL!");
				break;
			}
		}
	}
}