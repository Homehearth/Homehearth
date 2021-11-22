#include "GameSystems.h"
#include "EnginePCH.h"
#include "Healthbar.h"
#include "Game.h"

//System to render collider mesh red if collider is colliding with another collider
void GameSystems::RenderIsCollidingSystem(Scene& scene)
{
	scene.ForEachComponent<comp::RenderableDebug>([&](Entity entity, comp::RenderableDebug& renderableDebug)
		{
			//Collided with mouse TODO make it do someting?
			if (CollisionSystem::Get().GetCollisionCount(entity) > 0)
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
void GameSystems::UpdateHealthbar(Game* game)
{
	size_t i = game->m_players.size();

	Scene* scene = &game->GetScene("Game");

	scene->ForEachComponent<comp::Health, comp::Player>([&](Entity e, comp::Health& health, const comp::Player& player)
		{
			rtd::Healthbar* healthbar = dynamic_cast<rtd::Healthbar*>(scene->GetCollection("player" + std::to_string(i) + "Info")->elements[0].get());
			if (healthbar)
			{
				healthbar->SetHealthVariable(e);
			}
			i--;
		});
}

// Will check if a ray hits an object before it hits the player and if it does it will add the object to be rendered transparent
void GameSystems::CheckLOS(Game* game)
{
	comp::Transform* t = game->m_players.at(game->m_localPID).GetComponent<comp::Transform>();
	Camera* cam = game->GetCurrentScene()->GetCurrentCamera();

	if (t && cam->GetCameraType() == CAMERATYPE::PLAY)
	{
		sm::Vector3 playerPos = t->position;
		// Shoot a ray from cameras position to players position
		Ray_t ray;
		ray.origin = cam->m_position;
		ray.dir = (t->position - ray.origin);
		ray.dir.Normalize(ray.dir);

		for (int i = 0; i < game->m_LOSColliders.size(); i++)
		{
			if (ray.Intersects(game->m_LOSColliders[i].second))
			{
				for (int j = 0; j < game->m_models.at(game->m_LOSColliders[i].first).size(); j++)
				{
					//LOG_INFO("Behind an object!");
					game->m_models.at(game->m_LOSColliders[i].first)[j].GetComponent<comp::Renderable>()->isSolid = false;
				}
			}
		}
	}
}

static bool STRECH_ONCE = true;

void GameSystems::UpdatePlayerVisuals(Game* game)
{
	size_t i = game->m_players.size();
	Scene* scene = game->GetCurrentScene();

	scene->ForEachComponent<comp::Player, comp::Transform, comp::Network>([&](comp::Player& player, comp::Transform& t, comp::Network& n)
		{
			/*
				Own players health should be displayed at the lower left corner.
			*/
			if (n.id == game->m_localPID)
			{
				const float width = (float)game->GetWindow()->GetWidth();
				const float height = (float)game->GetWindow()->GetHeight();
				Scene* scene = &game->GetScene("Game");
				// Update healthbars position.
				Collection2D* collHealth = scene->GetCollection("player" + std::to_string(i) + "Info");
				if (collHealth)
				{
					rtd::Healthbar* health = dynamic_cast<rtd::Healthbar*>(collHealth->elements[0].get());
					if (health)
					{
						// Update healthbars position.
						if (STRECH_ONCE)
						{
							health->SetStretch(width / 3.33f, height / 16.f);
							STRECH_ONCE = false;
						}
						health->SetPosition(width / 32.0f, height - (height / 16.0f) - (height / 32.0f));
						health->SetVisiblity(true);
					}
				}
			}
			else
			{
				Collection2D* collection = scene->GetCollection("dynamicPlayer" + std::to_string(i) + "namePlate");
				if (collection)
				{
					rtd::Text* namePlate = dynamic_cast<rtd::Text*>(collection->elements[0].get());
					if (namePlate)
					{
						Camera* cam = scene->GetCurrentCamera();

						if (cam->GetCameraMatrixes())
						{
							// Conversion from World space to NDC space.
							sm::Vector4 oldP = { t.position.x, t.position.y + 21.f, t.position.z, 1.0f };
							sm::Vector4 newP = dx::XMVector4Transform(oldP, cam->GetCameraMatrixes()->view);
							newP = dx::XMVector4Transform(newP, cam->GetCameraMatrixes()->projection);
							newP.x /= newP.w;
							newP.y /= newP.w;
							newP.z /= newP.w;

							// Conversion from NDC space [-1, 1] to Window space
							float new_x = (((newP.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
							float new_y = D2D1Core::GetWindow()->GetHeight() - (((newP.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

							namePlate->SetPosition(new_x - ((namePlate->GetText().length() * D2D1Core::GetDefaultFontSize()) * 0.5f), new_y);
							// Show nameplates only if camera is turned to it.
							if (newP.z < 1.f)
								namePlate->SetVisiblity(true);
							else
								namePlate->SetVisiblity(false);

							// Update healthbars position.
							Collection2D* collHealth = scene->GetCollection("player" + std::to_string(i) + "Info");
							if (collHealth)
							{
								rtd::Healthbar* health = dynamic_cast<rtd::Healthbar*>(collHealth->elements[0].get());
								if (health)
								{
									sm::Vector4 oldPp = { t.position.x, t.position.y + 17.0f, t.position.z, 1.0f };
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
			i--;
		});
}
