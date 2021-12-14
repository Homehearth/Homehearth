#include "GameSystems.h"
#include "EnginePCH.h"
#include "Healthbar.h"
#include "Game.h"
#include "Picture.h"

void GameSystems::DisplayUpgradeDefences(Game* game)
{
	Collection2D* coll = game->GetScene("Game").GetCollection("priceTag");
	
	if (game->GetCycler().GetTimePeriod() != CyclePeriod::NIGHT)
	{
		// Display only if in Build mode..
		ShopItem shopitem = game->GetShopItem();
		if (shopitem == ShopItem::Defence1x1 || shopitem == ShopItem::Defence1x3)
		{
			Scene& scene = *game->GetCurrentScene();
			bool shouldNotShow = true;
			const unsigned int width = D2D1Core::GetWindow()->GetWidth();
			const unsigned int height = D2D1Core::GetWindow()->GetHeight();

			bool pressed = false;
			if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
			{
				pressed = true;
			}


			float t = 9999;
			rtd::Picture* pc = dynamic_cast<rtd::Picture*>(coll->elements[0].get());
			rtd::Text* tc = dynamic_cast<rtd::Text*>(coll->elements[1].get());
			uint32_t id;
			uint32_t cost = 0;

			int test = 0;
			Ray_t mouseRay = InputSystem::Get().GetMouseRay();
			scene.ForEachComponent<comp::OrientedBoxCollider, comp::Cost, comp::Network>([&](comp::OrientedBoxCollider& box, comp::Cost& c, comp::Network& n) {

				float nt;
				if (mouseRay.Intersects(box, &nt))
				{
					if (nt < t)
					{
						t = nt;
						id = n.id;
						cost = c.cost;
					}

					shouldNotShow = false;
				}

				test++;

				});


			//std::cout << "Defences: " << test << "\n";
			// Update the UI to reflect on the closest defence to the mouse pointer.
			if (pc && tc)
			{
				pc->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
				tc->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x + width * 0.019f, (FLOAT)InputSystem::Get().GetMousePos().y);
				tc->SetText("Cost: " + std::to_string(cost));
				if (game->GetMoney() < cost)
					pc->SetTexture("NotEnoughMoneySign.png");
				else
					pc->SetTexture("EnoughMoneySign.png");
				coll->Show();
				if (pressed)
				{
					game->UpgradeDefence(id);
				}
			}

			if (shouldNotShow)
				coll->Hide();
		}
		else
			coll->Hide();
	}
	else
		coll->Hide();
}

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
	Scene* scene = &game->GetScene("Game");

	scene->ForEachComponent<comp::Health, comp::Player>([&](Entity e, comp::Health& health, const comp::Player& player)
		{
			rtd::Healthbar* healthbar = dynamic_cast<rtd::Healthbar*>(scene->GetCollection("Aplayer" + std::to_string(static_cast<uint16_t>(player.playerType)) + "Info")->elements[0].get());
			if (healthbar)
			{
				healthbar->SetHealthVariable(e);
			}
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
	Scene& scene = game->GetScene("Game");
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();

	scene.ForEachComponent<comp::Player, comp::Transform, comp::Network>([&](comp::Player& player, comp::Transform& t, comp::Network& n)
		{
			/*
				Own players health should be displayed at the lower left corner.
			*/
			if (n.id == game->m_localPID)
			{
				Scene* scene = &game->GetScene("Game");
				// Update healthbars position.
				Collection2D* collHealth = scene->GetCollection("Aplayer" + std::to_string(static_cast<uint16_t>(player.playerType)) + "Info");
				if (collHealth)
				{
					rtd::Healthbar* health = dynamic_cast<rtd::Healthbar*>(collHealth->elements[0].get());
					if (health)
					{
						health->SetStretch(width / 4.0f, height / 24.f);
						health->SetPosition(width / 32.0f, height - (height / 13.0f));
						health->SetVisiblity(true);
					}
				}
				Collection2D* collection = scene->GetCollection("AdynamicPlayer" + std::to_string(static_cast<uint16_t>(player.playerType)) + "namePlate");
				if (collection)
				{
					collection->Hide();
				}
			}
			else
			{
				Collection2D* collection = scene.GetCollection("AdynamicPlayer" + std::to_string(static_cast<uint16_t>(player.playerType)) + "namePlate");
				if (collection)
				{
					rtd::Text* namePlate = dynamic_cast<rtd::Text*>(collection->elements[0].get());
					if (namePlate)
					{
						Camera* cam = scene.GetCurrentCamera();

						if (cam->GetCameraMatrixes())
						{
							// Conversion from World space to NDC space.
							const float offsetName = 21.f;
							sm::Vector4 oldP = { t.position.x, t.position.y + offsetName, t.position.z, 1.0f };
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
							Collection2D* collHealth = scene.GetCollection("Aplayer" + std::to_string(static_cast<uint16_t>(player.playerType)) + "Info");
							if (collHealth)
							{
								rtd::Healthbar* health = dynamic_cast<rtd::Healthbar*>(collHealth->elements[0].get());
								if (health)
								{
									const float offsetHPBar = 17.f;
									sm::Vector4 oldPp = { t.position.x, t.position.y + offsetHPBar, t.position.z, 1.0f };
									sm::Vector4 newPp = dx::XMVector4Transform(oldPp, cam->GetCameraMatrixes()->view);
									newPp = dx::XMVector4Transform(newPp, cam->GetCameraMatrixes()->projection);
									newPp.x /= newPp.w;
									newPp.y /= newPp.w;
									newPp.z /= newPp.w;

									// Conversion from NDC space [-1, 1] to Window space
									new_x = (((newPp.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
									new_y = D2D1Core::GetWindow()->GetHeight() - (((newPp.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

									health->SetStretch((width / 24), (height / 100));
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
		});
}

void GameSystems::DeathParticleTimer(Scene& scene)
{
	scene.ForEachComponent<comp::EmitterParticle>([&](Entity e, comp::EmitterParticle& emitter)
		{
			if (emitter.hasDeathTimer == true && emitter.lifeLived <= emitter.lifeTime)
			{
				emitter.lifeLived += Stats::Get().GetFrameTime();
			}
			else if (emitter.hasDeathTimer == true && emitter.lifeLived >= emitter.lifeTime)
				e.RemoveComponent<comp::EmitterParticle>();
		});
}
