#include "EnginePCH.h"
#include "Game.h"
#include "GameSystems.h"
#include "SceneHelper.h"
#include "Canvas.h"
#include "Picture.h"
#include "Border.h"
#include "Button.h"
#include "Text.h"
#include "TextField.h"
#include "Slider.h"
#include "Collection2D.h"
#include "Healthbar.h"
#include "Scroller.h"
#include <windows.h>
#include <shellapi.h>
#include "MoneyUI.h"
#include "AbilityUI.h"
#include "ShopUI.h"
#include "OptionSystem.h"
#include "MenuUI.h"

// Used to show and hide shopMenu
static bool toggle = false;


namespace sceneHelp
{
	Entity CreateLightEntity(Scene& scene, sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, float intensity, TypeLight type, UINT enabled)
	{
		Entity lightEntity = scene.CreateEntity();

		lightEntity.AddComponent<comp::Light>();
		lightEntity.GetComponent<comp::Light>()->lightData.position = pos;
		lightEntity.GetComponent<comp::Light>()->lightData.direction = dir;
		lightEntity.GetComponent<comp::Light>()->lightData.color = col;
		lightEntity.GetComponent<comp::Light>()->lightData.range = range;
		lightEntity.GetComponent<comp::Light>()->lightData.type = type;
		lightEntity.GetComponent<comp::Light>()->lightData.enabled = enabled;
		lightEntity.GetComponent<comp::Light>()->lightData.intensity = intensity;
		lightEntity.GetComponent<comp::Light>()->maxFlickerTime = (float)(rand() % 10 + 1) / 10.f;

		scene.GetLights()->EditLight(lightEntity.GetComponent<comp::Light>()->lightData, lightEntity.GetComponent<comp::Light>()->index);

		lightEntity.AddComponent<comp::SphereCollider>();


		return lightEntity;
	}

	void CreateMainMenuScene(Game* game)
	{
		// Scene logic
		Scene& mainMenuScene = game->GetScene("MainMenu");
		SetupMainMenuScreen(game);

		SoundHandler::Get().SetCurrentMusic("MenuTheme");

		mainMenuScene.on<ESceneUpdate>([](const ESceneUpdate& e, Scene& scene)
			{
				IMGUI(
					ImGui::Begin("Scene");
				ImGui::Text("MainMenu");
				ImGui::End();
				);
			});
	}

	void CreateLobbyScene(Game* game)
	{
		Scene& lobbyScene = game->GetScene("Lobby");
		SetupInLobbyScreen(game);

		lobbyScene.on<ESceneUpdate>([](const ESceneUpdate& e, Scene& scene)
			{
				IMGUI(
					ImGui::Begin("Scene");
				ImGui::Text("Lobby");
				ImGui::End();
				);
			});
	}

	void CreateJoinLobbyScene(Game* game)
	{
		Scene& joinLobbyScene = game->GetScene("JoinLobby");
		SetupLobbyJoinScreen(game);
	}

	void CreateLoadingScene(Game* game)
	{
		Scene& loadingScene = game->GetScene("Loading");
		SetupLoadingScene(game);
	}

	void CreateOptionsScene(Game* game)
	{
		Scene& optionsMenu = game->GetScene("Options");
		SetupOptionsScreen(game);
	}

	void CreateGameOverScene(Game* game)
	{
		Scene& gameOverMeny = game->GetScene("GameOver");
		SetupGameOverScreen(game);
	}

	void CreateGameScene(Game* game)
	{
		Scene& gameScene = game->GetScene("Game");
		gameScene.m_localPIDRef = &game->m_localPID;
		SetupInGameScreen(game);

		//Construct collider meshes if colliders are added.
		gameScene.GetRegistry()->on_construct<comp::RenderableDebug>().connect<entt::invoke<&comp::RenderableDebug::InitRenderable>>();
		gameScene.GetRegistry()->on_construct<comp::OrientedBoxCollider>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::SphereCollider>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::Light>().connect<&Lights::Add>(gameScene.GetLights());

		game->GetParticleSystem()->Initialize(D3D11Core::Get().Device());
		gameScene.GetRegistry()->on_construct<comp::EmitterParticle>().connect<&ParticleSystem::InitializeParticles>(game->GetParticleSystem());

		// Setup Cameras
		Entity debugCameraEntity = gameScene.CreateEntity();
		debugCameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(200, 60, -320), sm::Vector3(200, 50, -350), sm::Vector3(0, 1, 0),
			sm::Vector2((float)game->GetWindow()->GetWidth(), (float)game->GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
		debugCameraEntity.AddComponent<comp::Tag<TagType::DEBUG_CAMERA>>();

		Entity cameraEntity = gameScene.CreateEntity();
		comp::Camera3D* gameCamera = cameraEntity.AddComponent<comp::Camera3D>();
		gameCamera->camera.Initialize(sm::Vector3(60, 100.f, 80), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)game->GetWindow()->GetWidth(), (float)game->GetWindow()->GetHeight()), CAMERATYPE::PLAY);
		gameCamera->camera.SetFOV(dx::XMConvertToRadians(30.f));
		cameraEntity.AddComponent<comp::Tag<TagType::CAMERA>>();

		gameScene.SetCurrentCameraEntity(cameraEntity);

		float pointRange = 9.f;

		// The sun
		Entity sun = CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { -1.0f, 0.0f, -1.f, 0.f }, { 255.f, 185, 150, 0.f }, 1000.f, 0.09f, TypeLight::DIRECTIONAL, 1);
		// The moon
		Entity moon = CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { -1.0f, 0.0f, -1.f, 0.f }, { 50.f, 50, 200, 0.f }, 1000.f, 0.008f, TypeLight::DIRECTIONAL, 0);

		sm::Vector4 pointLightColor = { 237.f, 147.f, 18.f, 0.f };
		float pointIntensity = 0.f;

		// LEFT OF WELL
		CreateLightEntity(gameScene, { 268.2f, 28.f, -320.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, pointLightColor, pointRange, pointIntensity, TypeLight::POINT, 0);
		// FURTHEST LEFT AND FURTHEST SOUTH
		CreateLightEntity(gameScene, { 347.5f, 28.f, -323.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, pointLightColor, pointRange, pointIntensity, TypeLight::POINT, 0);
		// NEXT TO THE BRIDGE GOING SOUTH
		CreateLightEntity(gameScene, { 310.f, 28.f, -305.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, pointLightColor, pointRange, pointIntensity, TypeLight::POINT, 0);
		// NEXT TO THE LEFT BRIDGE ON THE LEFT SIDE OF IT
		CreateLightEntity(gameScene, { 307.f, 28.f, -350.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, pointLightColor, pointRange, pointIntensity, TypeLight::POINT, 0);
		// RIGHT OF THE WELL BETWEEN THE 2 HOUSES
		CreateLightEntity(gameScene, { 177.f, 28.f, -313.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, pointLightColor, pointRange, pointIntensity, TypeLight::POINT, 0);
		// FURTHEST RIGHT AND FURTHEST SOUTH
		CreateLightEntity(gameScene, { 193.5f, 28.f, -261.5f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, pointLightColor, pointRange, pointIntensity, TypeLight::POINT, 0);
		// LEFT OF WATERMILL
		CreateLightEntity(gameScene, { 338.5f, 28.f, -397.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, pointLightColor, pointRange, pointIntensity, TypeLight::POINT, 0);

		InputSystem::Get().SetCamera(gameScene.GetCurrentCamera());

		gameScene.on<ESceneUpdate>([=](const ESceneUpdate& e, Scene& scene)
			{
				scene.ForEachComponent<comp::Watermill>([=](Entity& entity, comp::Watermill& mill)
					{
						mill.theta += 45.f * e.dt;

						if (mill.theta >= 360.f)
						{
							mill.theta -= 360.f;
						}

						comp::Transform* t = entity.GetComponent<comp::Transform>();

						if (t)
						{
							t->rotation = sm::Quaternion::CreateFromAxisAngle({ 1,0,0 }, dx::XMConvertToRadians(-mill.theta));
						}
					});

				game->GetCycler().Update(e.dt);

				if (game->m_players.find(game->m_localPID) != game->m_players.end())
				{
					Camera* cam = scene.GetCurrentCamera();
					sm::Vector3 playerPos;
					if (cam->GetCameraType() == CAMERATYPE::PLAY)
					{
						playerPos = cam->GetTargetEntity().GetComponent<comp::Transform>()->position;
					}
					else
					{
						playerPos = cam->GetPosition();
					}

					comp::Light* l = sun.GetComponent<comp::Light>();

					float angle = 360.f * game->GetCycler().GetTime();
					for (int i = 0; i < 2; i++)
					{
						sm::Vector3 dir = sm::Vector3::TransformNormal(sm::Vector3(-1, 0, -1), sm::Matrix::CreateRotationZ(dx::XMConvertToRadians(angle)));
						l->lightData.direction = sm::Vector4(dir);
						l->lightData.direction.w = 0.0f;

						l->lightData.enabled = l->lightData.direction.y < 0.0f;

						sm::Vector3 newPos = playerPos - dir * 300;
						if (l->lightData.enabled)
							newPos = util::Lerp(sm::Vector3(l->lightData.position), newPos, e.dt * 10);

						l->lightData.position = sm::Vector4(newPos);
						l->lightData.position.w = 1.0f;


						l = moon.GetComponent<comp::Light>();
						angle -= 180.f;
					}
				}

				if (game->GetCycler().HasChangedPeriod())
				{
					switch (game->GetCycler().GetTimePeriod())
					{
					case CyclePeriod::NIGHT:
					{

						SoundHandler::Get().SetCurrentMusic("NightTheme");
						scene.ForEachComponent<comp::Light>([](comp::Light& l)
							{
								if (l.lightData.type == TypeLight::POINT)
								{
									l.lightData.enabled = 1;
								}
							});
						break;
					}
					case CyclePeriod::MORNING:
					{
						SoundHandler::Get().SetCurrentMusic("MenuTheme");

						scene.ForEachComponent<comp::Light>([](comp::Light& l)
							{
								if (l.lightData.type == TypeLight::POINT)
								{
									l.lightData.enabled = 0;
								}
							});
						break;
					}
					case CyclePeriod::EVENING:
					{

						break;
					}
					default:
						break;
					}
				}



				Collection2D* bullColl = game->GetScene("Game").GetCollection("bullDoze");

				rtd::Picture* bullIcon = dynamic_cast<rtd::Picture*>(bullColl->elements[0].get());
				ShopItem shopitem = game->GetShopItem();

				if (bullIcon)
				{
					if (shopitem == ShopItem::Destroy_Tool)
					{
						bullColl->Show();
						bullIcon->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
					}
					else
					{
						bullColl->Hide();
					}
				}

				//Not in buildmode
				if (!(shopitem == ShopItem::Defence1x1 || shopitem == ShopItem::Defence1x3))
				{

					if (game->GetCurrentScene()->GetCollection("shopMenu")->GetState() == ElementState::OUTSIDE &&
						game->GetCurrentScene()->GetCollection("ScrolldownMenu")->GetState() == ElementState::OUTSIDE)
					{
						if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED))
						{
							game->GetCurrentScene()->GetCollection("shopMenu")->Hide();
							game->SetShopItem(ShopItem::None);
							bullColl->Hide();
						}
					}
				}

				GameSystems::DeathParticleTimer(scene);

				GameSystems::DisplayUpgradeDefences(game);
				//GameSystems::RenderIsCollidingSystem(scene);
				GameSystems::UpdatePlayerVisuals(game);
				Systems::LightSystem(scene, e.dt);
				game->GetCurrentScene()->UpdateSkybox(game->GetCycler().GetTime());


				// Need to update Listener to make 3D sound work properly.
				Entity player;
				if (game->GetLocalPlayer(player))
				{
					const auto lookDir = DirectX::XMVector3Rotate({ 0.f, 0.f, -1.f },
						scene.GetCurrentCamera()->GetRotation());
					SoundHandler::Get().SetListenerPosition(player.GetComponent<comp::Transform>()->position, lookDir);
					SoundHandler::Get().Update();
				}

#ifdef _DEBUG
				if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
				{
					if (scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::DEBUG)
					{
						scene.SetCurrentCameraEntity(cameraEntity);
						scene.GetCurrentCamera()->SetNearFarPlane(40.f, 220.f);
						InputSystem::Get().SwitchMouseMode();
						LOG_INFO("Game Camera selected");
					}
					else if (scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
					{
						scene.SetCurrentCameraEntity(debugCameraEntity);
						scene.GetCurrentCamera()->SetNearFarPlane(0.1f, 800.f);
						if (InputSystem::Get().IsMouseRelative())
						{
							InputSystem::Get().SwitchMouseMode();
						}
						LOG_INFO("Debug Camera selected");
					}
				}
				if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED))
				{
					if (scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::DEBUG)
					{
						InputSystem::Get().SwitchMouseMode();
					}
				}

#endif // DEBUG
			});
	}

	void SetupMainMenuScreen(Game* game)
	{
		// Setup main menu scene.
		Scene& scene = game->GetScene("MainMenu");
		float width = (float)game->GetWindow()->GetWidth();
		float height = (float)game->GetWindow()->GetHeight();

		float widthScale = height * (16.f / 9.f);
		float padding;


		/*---------Background---------*/
		Collection2D* mainMenu = new Collection2D;
		mainMenu->AddElement<rtd::Picture>("MainMenu.png", draw_t(0, 0, width, height));
		scene.Add2DCollection(mainMenu, "AMainMenu");
		/*---------Background---------*/


		/*---------Textfields---------*/
		Collection2D* connectFields = new Collection2D;
		rtd::TextField* ipField = connectFields->AddElement<rtd::TextField>(draw_text_t((width / 2) - (widthScale * 0.25), height * 0.55f, widthScale * 0.25f, D2D1Core::GetDefaultFontSize()), 30, true);
		ipField->SetDescriptionText("IP Address:");
		padding = (widthScale / 64.f);
		rtd::TextField* portField = connectFields->AddElement<rtd::TextField>(draw_text_t((width / 2) + padding, height * 0.55f, widthScale * 0.25f, D2D1Core::GetDefaultFontSize()), 6);
		portField->SetDescriptionText("Port:");
		/*---------Textfields---------*/


		/*---------Buttons---------*/
		sm::Vector2 buttonSize = { widthScale / 6.f, height / 10.f };
		sm::Vector2 buttonPos = { (width / 2) - (buttonSize.x / 2),  height - (height * 0.37f) };
		padding = buttonSize.y + height * 0.02f;
		rtd::Button* connectButton = connectFields->AddElement<rtd::Button>("Start.png", draw_t(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y));
		buttonPos.y += padding;
		rtd::Button* settingsButton = connectFields->AddElement<rtd::Button>("Settings.png", draw_t(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y));
		buttonPos.y += padding;
		rtd::Button* quitButton = connectFields->AddElement<rtd::Button>("Quit.png", draw_t(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y)); //width / 4.f, height * 0.15f
		scene.Add2DCollection(connectFields, "ConnectFields");

		rtd::Button* externalLinkBtn = connectFields->AddElement<rtd::Button>("Button.png", draw_t(width - width / 4.f, height - (height / 5), width / 8.f, height / 16));
		externalLinkBtn->GetText()->SetScale(0.5f);
		externalLinkBtn->GetText()->SetText("Give Feedback!");
		externalLinkBtn->SetOnPressedEvent([] {
			ShellExecuteA(NULL, "open", "https://forms.gle/1E4f4a9jqKoNpCgZ7", NULL, NULL, SW_SHOWNORMAL);
			});

		rtd::Text* deadServerText = connectFields->AddElement<rtd::Text>("Error connecting to server", draw_text_t((width / 8.0f), (height / 8.0f) * 5.0f, width / 4.0f, height / 8.0f));
		deadServerText->SetVisiblity(false);

#ifdef _DEBUG
		ipField->SetPresetText("127.0.0.1");
#else
		ipField->SetPresetText("homehearth.ddns.net");
#endif
		portField->SetPresetText("4950");


		connectButton->SetOnPressedEvent([=]()
			{
								
				std::string* ipConnect = ipField->RawGetBuffer();
				std::string* portConnect = portField->RawGetBuffer();

				if (ipConnect->length() > 0 && portConnect->length() > 0)
				{
					if (game->m_client.Connect(ipConnect->c_str(), std::stoi(portConnect->c_str())))
					{
						rtd::TextField* nameInput = dynamic_cast<rtd::TextField*>(game->GetScene("JoinLobby").GetCollection("nameInput")->elements[0].get());
						nameInput->SetActive();
						game->SetScene("JoinLobby");
						deadServerText->SetVisiblity(false);

					}
					else
					{
						deadServerText->SetVisiblity(true);
					}
				}
				else
				{
					LOG_WARNING("Please enter a valid ip/port");
				}
			});

		settingsButton->SetOnPressedEvent([=]()
			{
				game->SetScene("Options");
			});
		quitButton->SetOnPressedEvent([=]()
			{
				game->Shutdown();
			});

		/*---------Buttons---------*/

	}

	/*
	* LICENSE
	<div>Icons made by <a href="https://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
	*/

	void SetupInGameScreen(Game* game)
	{
		//// Temp textures
		Scene& scene = game->GetScene("Game");
		const std::string& texture1 = "like.png";
		const std::string& texture2 = "swordUI.png";
		float width = (float)game->GetWindow()->GetWidth();
		float height = (float)game->GetWindow()->GetHeight();

		float widthScale = height * (16.f / 9.f);
		sm::Vector2 padding = { (widthScale / 64.f) , (widthScale / 64.f) / (16.f / 9.f) };

		// Picture that will be drawn when player is in destroy mode.
		Collection2D* bullDoze = new Collection2D;
		bullDoze->AddElement<rtd::Picture>("No.png", draw_t(0.0f, 0.0f, widthScale / 24, height / 14));
		bullDoze->Hide();
		scene.Add2DCollection(bullDoze, "bullDoze");

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			Collection2D* playerHp = new Collection2D;

			// Initiate 3 healthbars. for each player.
			playerHp->AddElement<rtd::Healthbar>(draw_t(width / 8, (i * ((height / 12)) + (height / 32)), (widthScale / 24), (height / 100)));

			// You and Friend text
			if (i == 0)
			{
			}
			else
			{
				playerHp->Hide();
			}
			scene.Add2DCollection(playerHp, "player" + std::to_string(i + 1) + "Info");
		}

		//Collection2D* timerCollection = new Collection2D;
		//timerCollection->AddElement<rtd::Text>("0", draw_text_t(0, 0, width, height / 16.f));
		//scene.Add2DCollection(timerCollection, "timer");

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			Collection2D* nameCollection = new Collection2D;
			nameCollection->AddElement<rtd::Text>("Player", draw_text_t(0, 0, widthScale / 14, height / 6));
			scene.Add2DCollection(nameCollection, "dynamicPlayer" + std::to_string(i + 1) + "namePlate");
			nameCollection->Hide();
		}

		Collection2D* money = new Collection2D;
		rtd::MoneyUI* mMoney = money->AddElement<rtd::MoneyUI>(draw_text_t(width - (widthScale / 8.0f + padding.x), padding.y, widthScale / 8.0f, height / 11.0f));
		scene.Add2DCollection(money, "MoneyUI");

		
		Collection2D* abilities = new Collection2D;
		sm::Vector2 barPos = {((width / 2.f)) - ((widthScale / 16.0f) * 2.5f), height - (height / 9.0f + padding.y)};
		rtd::Picture* abilityBar = abilities->AddElement<rtd::Picture>("AbilityBar.png", draw_t(barPos.x, barPos.y, (widthScale / 16.0f) * 5.0f, height / 9.0f));
		
		sm::Vector2 abillitySize = { widthScale / 18.0f, height / 11.0f };
		sm::Vector2 abillityPos = { (width / 2.f) - (abillitySize.x / 2), barPos.y + (padding.y * 0.5f)};

		rtd::AbilityUI* primary = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x - (abillitySize.x * 2 + padding.x), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "Attack2.png");
		primary->SetActivateButton("LMB");
		primary->SetReference(&game->m_primaryCooldown);
		rtd::AbilityUI* secondary = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x - (abillitySize.x + padding.x / 2), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "Block.png");
		secondary->SetActivateButton("RMB");
		secondary->SetReference(&game->m_secondaryCooldown);
		rtd::AbilityUI* third = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x, abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "Dodge.png");
		third->SetActivateButton("Shift");
		third->SetReference(&game->m_dodgeCooldown);
		rtd::AbilityUI* fourth = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x + (abillitySize.x + padding.x / 2), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "LockedIcon.png");
		rtd::AbilityUI* fith = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x + (abillitySize.x * 2 + padding.x), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "LockedIcon.png");
		scene.Add2DCollection(abilities, "AbilityUI");

		Collection2D* pauseMenu = new Collection2D;
		rtd::MenuUI* inGameMenu = pauseMenu->AddElement<rtd::MenuUI>("Menu.png", draw_t(width * 0.5f - (widthScale * 0.125f), (height / 2)  - (height * 0.25f), widthScale * 0.25f, height * 0.5f));
		inGameMenu->SetOnPressedEvent(0, [=]
			{
				game->Shutdown();
			});
		inGameMenu->SetOnPressedEvent(1, [=]
			{
				pauseMenu->Hide();
			});
		inGameMenu->SetOnPressedEvent(2, [=]
			{
				pauseMenu->Hide();
			});
		scene.Add2DCollection(pauseMenu, "inGameMenu");
		pauseMenu->Hide();

		Collection2D* shopMenu = new Collection2D;
		Collection2D* scrolldownMenu = new Collection2D;
		rtd::Scroller* sc = scrolldownMenu->AddElement<rtd::Scroller>(draw_t(0.0f, -(height / 14) * 3.0f, widthScale / 24.0f, (height / 16) * 3.0f), sm::Vector2(0, 0));
		sc->AddButton("No.png", draw_t(0.0f, -(height / 14), widthScale / 24, height / 14))->SetOnPressedEvent([=] {
			pauseMenu->Show();
			});


		sc->AddButton("ShopIcon.png", draw_t(0.0f, -(height / 14) * 2.0f, widthScale / 24, height / 14))->SetOnPressedEvent([=] {
			if (game->GetCycler().GetTimePeriod() == CyclePeriod::DAY)
			{
				shopMenu->Show();
				bullDoze->Hide();
			}
			});
		sc->SetPrimeButtonMeasurements(draw_t(0.0f, 0.0f, widthScale / 24, height / 14));
		sc->SetOnPrimeButtonPress([=] {

			shopMenu->Hide();

			});
		scene.Add2DCollection(scrolldownMenu, "ScrolldownMenu");

		rtd::ShopUI* shop = shopMenu->AddElement<rtd::ShopUI>("Shop.png", draw_t(width / 24.0f, (height / 16), widthScale * 0.25f, height * 0.5f));
		// 1x1 tower button.
		shop->SetOnPressedEvent(0, [=] {
			game->SetShopItem(ShopItem::Defence1x1);
			bullDoze->Hide();
			});
		// 1x3 tower button.
		shop->SetOnPressedEvent(1, [=] {
			game->SetShopItem(ShopItem::Defence1x3);
			bullDoze->Hide();
			});
		// Primary upgrade button.
		shop->SetOnPressedEvent(2, [=] {
			game->SetShopItem(ShopItem::Primary_Upgrade);
			bullDoze->Hide();
			});
		// Armor upgrade button.
		shop->SetOnPressedEvent(3, [=] {
			game->SetShopItem(ShopItem::Primary_Upgrade);
			bullDoze->Hide();
			});
		// Heal button.
		shop->SetOnPressedEvent(4, [=] {
			game->SetShopItem(ShopItem::Heal);
			bullDoze->Hide();
			});
		// Remove defences button.
		shop->SetOnPressedEvent(5, [=] {
			game->SetShopItem(ShopItem::Destroy_Tool);
			bullDoze->Show();
			});
		shop->SetMoneyRef(mMoney);
		shopMenu->Hide();
		scene.Add2DCollection(shopMenu, "shopMenu");

		Collection2D* priceTag = new Collection2D;
		priceTag->AddElement<rtd::Picture>("EnoughMoneySign.png", draw_t(0.0f, 0.0f, widthScale * 0.15f, height * 0.075f));
		priceTag->AddElement<rtd::Text>("Cost: UNK", draw_t(0.0f, 0.0f, widthScale * 0.15f, height * 0.075f));
		priceTag->Hide();
		scene.Add2DCollection(priceTag, "priceTag");

		Collection2D* spreeText = new Collection2D;
		spreeText->AddElement<rtd::Text>("X1", draw_t(width - (width / 12.f), height - (height / 8.0f), widthScale / 12.f, height / 8.0f));
		scene.Add2DCollection(spreeText, "SpreeText");
	}

	void SetupInLobbyScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();

		const float widthScale = height * (16.f / 9.f);
		const float paddingWidth = (widthScale / 64.f);
		const float paddingHeight = paddingWidth / (16.f / 9.f);

		Scene& scene = game->GetScene("Lobby");

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			float lobbyHeightPos = ((height / 12) + paddingHeight) * (i + 1) + paddingHeight * i;
			float lobbyWidthPos = width / 16;
			sm::Vector2 scale = { widthScale / 4, height / 9 };
			sm::Vector2 classIconPos = { lobbyWidthPos + scale.x + paddingWidth, lobbyHeightPos };

			Collection2D* playerIcon = new Collection2D;
			playerIcon->AddElement<rtd::Picture>("Button.png", draw_t(lobbyWidthPos, lobbyHeightPos, scale.x, scale.y));
			playerIcon->AddElement<rtd::Text>("Player " + std::to_string(i + 1), draw_text_t(lobbyWidthPos, lobbyHeightPos, scale.x, scale.y));
			playerIcon->AddElement<rtd::Picture>("WarriorIcon.png", draw_t(classIconPos.x, classIconPos.y, scale.x / 4, scale.y));
			playerIcon->AddElement<rtd::Picture>("Yes.png", draw_t(classIconPos.x + paddingWidth + (scale.x / 4), lobbyHeightPos, scale.x / 4, scale.y))->SetVisiblity(false);
			scene.Add2DCollection(playerIcon, "playerIcon" + std::to_string(i + 1));
		}

		Collection2D* classTextCanvas = new Collection2D;
		rtd::Picture* desc = classTextCanvas->AddElement<rtd::Picture>("WarriorDesc.png", draw_t(width - ((widthScale / 3.33f) + paddingWidth), 10.0f, (widthScale / 3.33f), height - (height / 6.0f)));
		scene.Add2DCollection(classTextCanvas, "ClassTextCanvas");

		Collection2D* lobbyDesc = new Collection2D;
		lobbyDesc->AddElement<rtd::Picture>("Button.png", draw_t((width / 16), height - (height / 6), (widthScale / 4), height / 9));

		// THIS ONE NEEDS A FUNCTION TO UPDATE LOBBY ID
		const std::string& lobbyString = "Lobby ID: XYZW";
		lobbyDesc->AddElement<rtd::Text>(lobbyString, draw_text_t((width / 16), height - (height / 6), (widthScale / 4), height / 9));
		scene.Add2DCollection(lobbyDesc, "LobbyDesc");

		Collection2D* startGame = new Collection2D;
		rtd::Button* startGameButton = startGame->AddElement<rtd::Button>("Ready.png", draw_t(width - ((widthScale / 3.33f) + paddingWidth), height - (height / 5.0f), (widthScale / 3.33f), (height / 6.f)), false);
		//rtd::Text* readyText = startGame->AddElement<rtd::Text>("Not ready", draw_text_t((width / 2) + (width / 10.f), height - (height / 5.0f), (width / 3.33f), (height / 6.f)));
		startGameButton->SetOnPressedEvent([=]()
			{
				Entity e;
				if (game->GetLocalPlayer(e))
				{
					comp::Player* p = e.GetComponent<comp::Player>();
					p->isReady = !p->isReady;
					game->SendStartGame();
				}
			});
		scene.Add2DCollection(startGame, "StartGame");

		Collection2D* general = new Collection2D;
		general->AddElement<rtd::Picture>("MenuBG.png", draw_t(0, 0, width, height));
		rtd::Button* exitButton = general->AddElement<rtd::Button>("No.png", draw_t(paddingWidth, paddingHeight, widthScale / 24, height / 14), false);
		exitButton->SetOnPressedEvent([=]()
			{
				Entity e;
				if (game->GetLocalPlayer(e))
				{
					comp::Player* p = e.GetComponent<comp::Player>();
					p->isReady = false;
					network::message<GameMsg> msg;
					msg.header.id = GameMsg::Lobby_Leave;
					msg << game->m_localPID << game->m_gameID;
					game->m_client.Send(msg);
					rtd::TextField* nameInput = dynamic_cast<rtd::TextField*>(game->GetScene("JoinLobby").GetCollection("nameInput")->elements[0].get());
					nameInput->SetActive();
				}
			});
		scene.Add2DCollection(general, "AGeneral");

		Collection2D* classButtons = new Collection2D;
		//rtd::Picture* warriorBorder = classButtons->AddElement<rtd::Picture>("Selected.png", draw_t((width / 3.33f) + (width / 15.0f) + (float)(width / 16) - 7.5f, height - (height / 6) - 7.5f, (width / 16.f) * 1.15f, (height / 9.f) * 1.15f));
		//rtd::Picture* mageBorder = classButtons->AddElement<rtd::Picture>("Selected.png", draw_t((width / 3.33f) + (float)(width / 20) - 7.5f, height - (height / 6) - 7.5f, (width / 16.f) * 1.15f, (height / 9.f) * 1.15f));
		sm::Vector2 classButtonSize = { widthScale / 16, height / 9 };
		float classButtonPosY = height - (classButtonSize.y + paddingHeight);
		rtd::Button* mageButton = classButtons->AddElement<rtd::Button>("WizardIcon.png", draw_t((width / 2) - ((classButtonSize.x + paddingWidth) / 2), classButtonPosY, classButtonSize.x, classButtonSize.y));
		rtd::Button* warriorButton = classButtons->AddElement<rtd::Button>("WarriorIcon.png", draw_t((width / 2) + ((classButtonSize.x + paddingWidth) / 2), classButtonPosY, classButtonSize.x, classButtonSize.y));
		warriorButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 1.0f, 0.2f));
		warriorButton->GetBorder()->SetLineWidth(LineWidth::THICC);
		warriorButton->GetBorder()->SetVisiblity(true);

		mageButton->GetBorder()->SetLineWidth(LineWidth::THICC);
		mageButton->GetBorder()->SetVisiblity(false);
		//mageBorder->SetVisiblity(false);
		// FIX WHAT CLASS SYMBOL PLAYER HAS LATER
		mageButton->SetOnPressedEvent([=]()
			{
				//mageBorder->SetVisiblity(true);
				//warriorBorder->SetVisiblity(false);
				desc->SetTexture("WizardDesc.png");
				Entity e;
				if (game->GetLocalPlayer(e))
				{
					comp::Player* p = e.GetComponent<comp::Player>();
					p->classType = comp::Player::Class::MAGE;
					game->SendSelectedClass(p->classType);
					mageButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 1.0f, 0.2f));
					mageButton->GetBorder()->SetVisiblity(true);
					warriorButton->GetBorder()->SetVisiblity(false);
				}
			});


		warriorButton->SetOnPressedEvent([=]()
			{
				//mageBorder->SetVisiblity(false);
				//warriorBorder->SetVisiblity(true);
				desc->SetTexture("WarriorDesc.png");
				Entity e;
				if (game->GetLocalPlayer(e))
				{
					comp::Player* p = e.GetComponent<comp::Player>();
					p->classType = comp::Player::Class::WARRIOR;
					game->SendSelectedClass(p->classType);
					warriorButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 1.0f, 0.2f));
					warriorButton->GetBorder()->SetVisiblity(true);
					mageButton->GetBorder()->SetVisiblity(false);
				}
			});

		scene.Add2DCollection(classButtons, "ClassButtons");
	}

	void SetupOptionsScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();
		Scene& scene = game->GetScene("Options");

		Collection2D* helpText = new Collection2D;
		Collection2D* menuBG = new Collection2D;

		menuBG->AddElement<rtd::Picture>("MenuBG.png", draw_t(0, 0, width, height));
		scene.Add2DCollection(menuBG, "AMenuBG");

		Collection2D* soundCollection = new Collection2D;
		rtd::Slider* sl = soundCollection->AddElement<rtd::Slider>(D2D1::ColorF(0.0f, 0.0f, 0.0f), draw_t((width / 2) - (width / 9), height / 5, width / 9, height / 16), &game->m_masterVolume, 1.0f, 0.0f);
		sl->SetMinPos(sm::Vector2((width / 8) - (width / 9)));
		sl->SetMaxPos(sm::Vector2(width - (width / 8)));
		sl->SetExplanationText("Master Volume: ");
		soundCollection->Hide();
		scene.Add2DCollection(soundCollection, "Sounds");

		// Declaration
		Collection2D* backButton = new Collection2D;
		Collection2D* visualMenu = new Collection2D;
		Collection2D* resolutionMenu = new Collection2D;
		Collection2D* miscMenu = new Collection2D;

		Collection2D* menu = new Collection2D;
		rtd::Button* soundsButton = menu->AddElement<rtd::Button>("Button.png", draw_t(width / 8.0f, height / 8.0f, width / 4.0f, height / 8.0f));
		rtd::Button* visualButton = menu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f) * 5.0f, (height / 8.0f), width / 4.0f, height / 8.0f));
		rtd::Button* helpButton = menu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f) * 5.0f, (height / 8.0f) * 4.0f, width / 4.0f, height / 8.0f));
		helpButton->SetOnPressedEvent([=] {

			helpText->Show();
			backButton->Show();
			menu->Hide();

			});
		menu->AddElement<rtd::Text>("Help", draw_text_t((width / 8.0f) * 5.0f, (height / 8.0f) * 4.0f, width / 4.0f, height / 8.0f));

		rtd::Button* returnTo = menu->AddElement<rtd::Button>("Button.png", draw_t((width / 2.0f) - (width / 8.0f), height - (height / 4.0f), width / 4.0f, height / 8.0f));
		menu->AddElement<rtd::Text>("Go Back", draw_text_t((width / 2.0f) - (width / 8.0f), height - (height / 4.0f), width / 4.0f, height / 8.0f));
		returnTo->SetOnPressedEvent([=] {

			game->SetScene("MainMenu");
			SoundHandler::Get().SetCurrentMusic("MenuTheme");

			});

		menu->AddElement<rtd::Text>("Sounds", draw_text_t(width / 8.0f, height / 8.0f, width / 4.0f, height / 8.0f));

		visualButton->SetOnPressedEvent([=] {

			menu->Hide();
			backButton->Show();
			visualMenu->Show();

			});
		menu->AddElement<rtd::Text>("Visuals", draw_text_t((width / 8.0f) * 5.0f, (height / 8.0f), width / 4.0f, height / 8.0f));
		soundsButton->SetOnPressedEvent([=]() {

			soundCollection->Show();
			menu->Hide();
			backButton->Show();

			});

		visualMenu->AddElement<rtd::Button>("Button.png", draw_t(width / 8.0f, height / 8.0f, width / 4.0f, height / 8.0f))->SetOnPressedEvent([=] {

			resolutionMenu->Show();
			visualMenu->Hide();

			});
		visualMenu->AddElement<rtd::Text>("Resolution", draw_t(width / 8.0f, height / 8.0f, width / 4.0f, height / 8.0f));
		visualMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f), (height / 8.0f) * 4.0f, width / 4.0f, height / 8.0f))->SetOnPressedEvent([=] {
			visualMenu->Hide();
			miscMenu->Show();
			});
		visualMenu->AddElement<rtd::Text>("Misc.", draw_t((width / 8.0f), (height / 8.0f * 4.0f), width / 4.0f, height / 8.0f));

		// Misc Menu options.

		// Blur option.
		static DoFType type = DoFType::DEFAULT;
		std::string savedBlur = OptionSystem::Get().GetOption("BlurType");
		type = static_cast<DoFType>(std::stod(savedBlur));
		rtd::Button* blurButton = miscMenu->AddElement<rtd::Button>("Button.png", draw_t(width / 8.0f, height / 8.0f, width / 4.0f, height / 8.0f));
		rtd::Text* blurType = miscMenu->AddElement<rtd::Text>("Blur Type: Adaptive", draw_t(width / 8.0f, height / 8.0f, width / 4.0f, height / 8.0f));

		// Toggle when the game starts.
		switch (type)
		{
		case DoFType::VIGNETTE:
		{
			blurType->SetText("Blur Type: Static");
			break;
		}
		case DoFType::DEFAULT:
		{
			blurType->SetText("Blur Type: NONE");
			break;
		}
		case DoFType::ADAPTIVE:
		{
			blurType->SetText("Blur Type: Adaptive");
			break;
		}
		default:
		{
			type = DoFType::DEFAULT;
			break;
		}
		}
		OptionSystem::Get().SetOption("BlurType", std::to_string(static_cast<int>(type)));

		blurButton->SetOnPressedEvent([=] {
			// Toggle between types.
			switch (type)
			{
			case DoFType::ADAPTIVE:
			{
				type = DoFType::VIGNETTE;
				blurType->SetText("Blur Type: Static");
				thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::VIGNETTE);
				OptionSystem::Get().SetOption("BlurType", std::string("2"));
				break;
			}
			case DoFType::VIGNETTE:
			{
				type = DoFType::DEFAULT;
				blurType->SetText("Blur Type: NONE");
				thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::DEFAULT);
				OptionSystem::Get().SetOption("BlurType", std::string("0"));
				break;
			}
			case DoFType::DEFAULT:
			{
				type = DoFType::ADAPTIVE;
				blurType->SetText("Blur Type: Adaptive");
				thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::ADAPTIVE);
				OptionSystem::Get().SetOption("BlurType", std::string("1"));
				break;
			}
			default:
				break;
			}

			});

		// Shadows toggle.
		static int lightQuality = std::stoi(OptionSystem::Get().GetOption("VolumetricLightQuality"));
		rtd::Button* lightQualityButton = miscMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f) * 5.0f, height / 8.0f, width / 4.0f, height / 8.0f));
		rtd::Text* lightQualityType = miscMenu->AddElement<rtd::Text>("Volumetric Light Quality: LOW", draw_t((width / 8.0f) * 5.0f, height / 8.0f, width / 4.0f, height / 8.0f));

		if (lightQuality <= 15 && lightQuality > 0)
		{
			lightQualityType->SetText("Volumetric Light Quality: LOW");
		}
		else if (lightQuality > 15 && lightQuality <= 50)
		{
			lightQualityType->SetText("Volumetric Light Quality: MEDIUM");
		}
		else if (lightQuality > 50 && lightQuality <= 100)
		{
			lightQualityType->SetText("Volumetric Light Quality: HIGH");
		}
		else if (lightQuality > 100 && lightQuality <= 200)
		{
			lightQualityType->SetText("Volumetric Light Quality: INSANE");
		}
		else
		{
			lightQualityType->SetText("Volumetric Light Quality: MEDIUM");
			lightQuality = 50;
		}

		game->GetScene("Game").GetLights()->SetLightVolumeQuality(lightQuality);
		OptionSystem::Get().SetOption("VolumetricLightQuality", std::to_string(lightQuality));

		lightQualityButton->SetOnPressedEvent([=] {

			switch (lightQuality)
			{
			case 15:
			{
				lightQuality = 50;
				lightQualityType->SetText("Volumetric Light Quality: MEDIUM");
				break;
			}
			case 50:
			{
				lightQuality = 100;
				lightQualityType->SetText("Volumetric Light Quality: HIGH");
				break;
			}
			case 100:
			{
				lightQuality = 200;
				lightQualityType->SetText("Volumetric Light Quality: INSANE");
				break;
			}
			case 200:
			{
				lightQuality = 15;
				lightQualityType->SetText("Volumetric Light Quality: LOW");
				break;
			}
			default:
			{
				lightQuality = 15;
				lightQualityType->SetText("Volumetric Light Quality: LOW");
				break;
			}
			};

			OptionSystem::Get().SetOption("VolumetricLightQuality", std::to_string(lightQuality));
			game->GetScene("Game").GetLights()->SetLightVolumeQuality(lightQuality);

			});

		// Shadow Quality Settings

		static std::string shadowQuality = OptionSystem::Get().GetOption("ShadowQuality");
		rtd::Button* shadowSizeButton = miscMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f) * 5.0f, (height / 8.0f) * 4.0f, width / 4.0f, height / 8.0f));
		rtd::Text* shadowSize = miscMenu->AddElement<rtd::Text>("Shadows Quality: MEDIUM", draw_t((width / 8.0f) * 5.0f, (height / 8.0f) * 4.0f, width / 4.0f, height / 8.0f));

		if (shadowQuality == "Potato")
		{
			shadowSize->SetText("Shadows Quality: POTATO");
		}
		else if (shadowQuality == "Low")
		{
			shadowSize->SetText("Shadows Quality: LOW");
		}
		else if (shadowQuality == "Medium")
		{
			shadowSize->SetText("Shadows Quality: MEDIUM");
		}
		else if (shadowQuality == "High")
		{
			shadowSize->SetText("Shadows Quality: HIGH");
		}
		else if (shadowQuality == "Insane")
		{
			shadowSize->SetText("Shadows Quality: INSANE");
		}
		else
		{
			shadowSize->SetText("Shadows Quality: MEDIUM");
			shadowQuality = "Medium";
			OptionSystem::Get().SetOption("ShadowQuality", std::string("Medium"));
		}

		shadowSizeButton->SetOnPressedEvent([=] {

			if (shadowQuality == "Potato")
			{
				shadowSize->SetText("Shadows Quality: LOW");
				shadowQuality = "Low";
				thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(256);
			}
			else if (shadowQuality == "Low")
			{
				shadowSize->SetText("Shadows Quality: MEDIUM");
				shadowQuality = "Medium";
				thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(1024);
			}
			else if (shadowQuality == "Medium")
			{
				shadowSize->SetText("Shadows Quality: HIGH");
				shadowQuality = "High";
				thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(2048);
			}
			else if (shadowQuality == "High")
			{
				shadowSize->SetText("Shadows Quality: INSANE");
				shadowQuality = "Insane";
				thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(4096);
			}
			else if (shadowQuality == "Insane")
			{
				shadowSize->SetText("Shadows Quality: POTATO");
				shadowQuality = "Potato";
				thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(512);
			}
			OptionSystem::Get().SetOption("ShadowQuality", shadowQuality);

			});


		/*
			Window Size Options
		*/

		static int winWidth = std::stoi(OptionSystem::Get().GetOption("WindowWidth"));
		static int winHeight = std::stoi(OptionSystem::Get().GetOption("WindowHeight"));
		rtd::Button* windowSizeButton = resolutionMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f) * 5.0f, (height / 8.0f) * 4.0f, width / 4.0f, height / 8.0f));
		rtd::Text* windowSize = resolutionMenu->AddElement<rtd::Text>("Window Size: 1920x1080", draw_t((width / 8.0f) * 5.0f, (height / 8.0f) * 4.0f, width / 4.0f, height / 8.0f));

		rtd::Button* tipSizeButton = resolutionMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f) * 5.0f, (height / 8.0f), width / 4.0f, height / 8.0f));
		rtd::Text* tipSizeText = resolutionMenu->AddElement<rtd::Text>("A restart is required for effects to change.", draw_t((width / 8.0f) * 5.0f, (height / 8.0f), width / 4.0f, height / 8.0f));

		switch (winWidth)
		{
		case 1920:
		{
			windowSize->SetText("Window Size: 1920x1080");
			break;
		}
		case 2560:
		{
			windowSize->SetText("Window Size: 2560x1440");
			break;
		}
		case 1408:
		{
			windowSize->SetText("Window Size: 1408x792");
			break;
		}
		case 1536:
		{
			windowSize->SetText("Window Size: 1536x864");
			break;
		}
		case 1632:
		{
			windowSize->SetText("Window Size: 1632x918");
			break;
		}
		default:
		{
			winWidth = 1920;
			winHeight = 1080;
			break;
		}
		};

		windowSizeButton->SetOnPressedEvent([=] {

			switch (winWidth)
			{
			case 1920:
			{
				windowSize->SetText("Window Size: 2560x1440");
				winWidth = 2560;
				winHeight = 1440;
				break;
			}
			case 2560:
			{
				windowSize->SetText("Window Size: 1408x792");
				winWidth = 1408;
				winHeight = 792;
				break;
			}
			case 1408:
			{
				windowSize->SetText("Window Size: 1536x864");
				winWidth = 1536;
				winHeight = 864;
				break;
			}
			case 1536:
			{
				windowSize->SetText("Window Size: 1632x918");
				winWidth = 1632;
				winHeight = 918;
				break;
			}
			case 1632:
			{
				windowSize->SetText("Window Size: 1920x1080");
				winWidth = 1920;
				winHeight = 1080;
				break;
			}
			default:
			{
				winWidth = 1920;
				winHeight = 1080;
				break;
			}
			};

			OptionSystem::Get().SetOption("WindowWidth", std::to_string(winWidth));
			OptionSystem::Get().SetOption("WindowHeight", std::to_string(winHeight));

			});

		rtd::Button* windowToggleButton = resolutionMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f), (height / 8.0f), width / 4.0f, height / 8.0f));
		rtd::Text* windowToggle = resolutionMenu->AddElement<rtd::Text>("Fullscreen", draw_t((width / 8.0f), (height / 8.0f), width / 4.0f, height / 8.0f));
		static int fullscreen = std::stoi(OptionSystem::Get().GetOption("Fullscreen"));

		if (fullscreen == 0)
		{
			windowToggle->SetText("Windowed");
		}
		else
		{
			windowToggle->SetText("Fullscreen");
		}

		windowToggleButton->SetOnPressedEvent([=] {

			if (fullscreen == 0)
			{
				windowToggle->SetText("Fullscreen");
				fullscreen = 1;
			}
			else
			{
				windowToggle->SetText("Windowed");
				fullscreen = 0;
			}

			OptionSystem::Get().SetOption("Fullscreen", std::to_string(fullscreen));

			});

		miscMenu->Hide();
		resolutionMenu->Hide();
		visualMenu->Hide();
		scene.Add2DCollection(miscMenu, "miscMenu");
		scene.Add2DCollection(resolutionMenu, "resolutionMenu");
		scene.Add2DCollection(visualMenu, "visualMenu");
		scene.Add2DCollection(menu, "MenuButtons");

		rtd::Button* gb = backButton->AddElement<rtd::Button>("Button.png", draw_t((width / 2.0f) - (width / 8.0f), height - (height / 4.0f), width / 4.0f, height / 8.0f));
		backButton->AddElement<rtd::Text>("Go Back", draw_text_t((width / 2.0f) - (width / 8.0f), height - (height / 4.0f), width / 4.0f, height / 8.0f));
		gb->SetOnPressedEvent([=] {

			helpText->Hide();
			soundCollection->Hide();
			menu->Show();
			backButton->Hide();
			visualMenu->Hide();
			miscMenu->Hide();
			resolutionMenu->Hide();

			});
		backButton->Hide();
		scene.Add2DCollection(backButton, "returnButton");

		const std::string helpTextOption = "Homehearth\n\n\nDuring the day its recommended to build defences around the village.\nTo do this you need to open the shop at upper right corner and press shop icon.\nTo place a defence press on one of the defence choices and click on any valid ground area. Other shop options are as presented.\nDuring the night you will need to work with your friends to defend the village from horrifying hordes of monsters.\nControls:\nWASD - Movement\nLeft Click - Primary Attack\nRight Click - Secondary Ability\nShift - Dodge\nTo place defences choose one in the shop and press Left click on any available slot. Right click to stop placing defences.";
		helpText->AddElement<rtd::Text>(helpTextOption, draw_text_t(0.0f, 0.0f, width, height - (height / 8.0f)));
		helpText->Hide();
		scene.Add2DCollection(helpText, "HelpText");

	}

	void SetupLoadingScene(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();
		Scene& scene = game->GetScene("Loading");

		Collection2D* loadingScreen = new Collection2D;

		loadingScreen->AddElement<rtd::Picture>("LoadingScreen.png", (draw_t(0.0f, 0.0f, width, height)));

		scene.Add2DCollection(loadingScreen, "LoadingScreen");
	}

	void SetupGameOverScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();

		float widthScale = height * (16.f / 9.f);
		Scene& scene = game->GetScene("GameOver");

		Collection2D* gameOverCollection = new Collection2D;

		rtd::Picture* bg = gameOverCollection->AddElement<rtd::Picture>("MenuBG.png", draw_t(0,0,width, height));
		rtd::Text* gameOverField = gameOverCollection->AddElement<rtd::Text>("Game Over", draw_text_t((width / 2.f) - (strlen("Game Over") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 5.f) - D2D1Core::GetDefaultFontSize(), strlen("Game Over") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));
		gameOverField->SetText("Game Over");

		rtd::Text* scoreField = gameOverCollection->AddElement<rtd::Text>("Score: 0", draw_text_t((width / 4.f) - (strlen("Score: ") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 2.f) - D2D1Core::GetDefaultFontSize(), strlen("Score: ") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));

		rtd::Text* waveField = gameOverCollection->AddElement<rtd::Text>("Waves: 0", draw_text_t((width / 1.5f) - (strlen("Waves: ") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 2.f) - D2D1Core::GetDefaultFontSize(), strlen("Waves: ") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));

		rtd::Button* mainMenuButton = gameOverCollection->AddElement<rtd::Button>("Button.png", draw_t((width / 2) - (widthScale / 8), height - (height / 6.f), widthScale / 4, height / 8));
		gameOverCollection->AddElement<rtd::Text>("Main Menu", draw_text_t((width / 2) - (width / 8), height - (height / 6.f), width / 4, height / 8));
		mainMenuButton->SetOnPressedEvent([=]
			{
				game->SetScene("JoinLobby");
				game->m_gameID = -1;
			});

		scene.Add2DCollection(gameOverCollection, "GameOver");
	}

	void SetupLobbyJoinScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();

		float widthScale = height * (16.f / 9.f);
		const float paddingWidth = (widthScale / 64.f);
		const float paddingHeight = paddingWidth / (16.f / 9.f);

		Scene& scene = game->GetScene("JoinLobby");


		Collection2D* nameCollection = new Collection2D;
		rtd::TextField* nameInputField = nameCollection->AddElement<rtd::TextField>(draw_text_t((width / 2) - (widthScale / 8), height / 8, widthScale / 4, D2D1Core::GetDefaultFontSize()), 12, true);
		nameInputField->SetDescriptionText("Input Name");
		rtd::Text* nameErrorText = nameCollection->AddElement<rtd::Text>("Invalid Name", draw_text_t((width / 2.0f) - (width / 8), (height / 8.0f) * 1.5f, widthScale / 4.0f, height / 8.0f));
		nameErrorText->SetVisiblity(false);
#ifdef _DEBUG
		nameInputField->SetPresetText("Player");
#endif // DEBUG

		scene.Add2DCollection(nameCollection, "nameInput");

		Collection2D* lobbyCollection = new Collection2D;
		lobbyCollection->AddElement<rtd::Picture>("MenuBG.png", draw_t(0, 0, width, height));

		sm::Vector2 buttonSize = { widthScale / 4.f, height * 0.15f };
		float buttonPosY = height - (buttonSize.y + paddingHeight * 2.f);

		rtd::Button* startLobbyButton = lobbyCollection->AddElement<rtd::Button>("CreateLobby.png", draw_t((width / 2.0f) + (width / 8.0f), buttonPosY, buttonSize.x, buttonSize.y));
		rtd::Button* lobbyButton = lobbyCollection->AddElement<rtd::Button>("joinLobby.png", draw_t(width / 8, buttonPosY, buttonSize.x, buttonSize.y));
		rtd::TextField* lobbyField = lobbyCollection->AddElement<rtd::TextField>(draw_text_t(width / 8, buttonPosY - paddingHeight * 4.f, widthScale / 4, D2D1Core::GetDefaultFontSize()));
		lobbyField->SetDescriptionText("Input Lobby ID");

		rtd::Button* exitButton = lobbyCollection->AddElement<rtd::Button>("No.png", draw_t(paddingWidth, paddingHeight, widthScale / 24, height / 14));
		rtd::Text* lobbyErrorText = lobbyCollection->AddElement<rtd::Text>("Invalid Lobby ID", draw_text_t(width / 8, height - (height / 3.33f), width / 4.0f, height / 8.0f));
		lobbyErrorText->SetVisiblity(false);
		exitButton->SetOnPressedEvent([=]
			{
				game->m_client.Disconnect();
			});

		startLobbyButton->SetOnPressedEvent([=]
			{
				if (nameInputField->RawGetBuffer()->length() > 0)
				{
					game->m_playerName = *nameInputField->RawGetBuffer();

					game->CreateLobby();

					// Update own name.
					dynamic_cast<rtd::Text*>(game->GetScene("Lobby").GetCollection("playerIcon1")->elements[1].get())->SetText(game->m_playerName);
				}
				else
				{
					nameErrorText->SetVisiblity(true);
					LOG_WARNING("Enter a valid nickname");
				}
			});
		lobbyButton->SetOnPressedEvent([=]()
			{
				std::string* lobbyString = lobbyField->RawGetBuffer();

				if (lobbyString)
				{
					game->m_playerName = *nameInputField->RawGetBuffer();
					int lobbyID = -1;
					try
					{
						lobbyID = std::stoi(*lobbyString);
					}
					catch (std::exception e)
					{
						lobbyErrorText->SetVisiblity(true);
						LOG_WARNING("Request denied: Invalid lobby ID: Was not numerical");
					}

					if (lobbyID > -1)
					{
						if (nameInputField->RawGetBuffer()->length() > 0)
						{
							game->JoinLobby(lobbyID);
							// Update own name.
							dynamic_cast<rtd::Text*>(game->GetScene("Lobby").GetCollection("playerIcon1")->elements[1].get())->SetText(game->m_playerName);
						}
						else
						{
							nameErrorText->SetVisiblity(true);
							LOG_WARNING("Request denied: Enter a valid nickname");
						}
					}
				}
			});

		scene.Add2DCollection(lobbyCollection, "LobbyFields");

	}

	bool LoadMapColliders(Game* game)
	{
		std::fstream file;

		file.open(BOUNDSLOADER);

		if (!file.is_open())
		{
			return false;
		}

		int nrOf = 0;
		while (!file.eof())
		{
			std::string filename;
			file >> filename;
			std::string colliderPath = BOUNDSPATH + filename;
			ModelID modelID;

			// Remove .obj / .fbx
			size_t count = filename.find_last_of('.');
			filename = filename.substr(0, count);

			if (House5 == filename)
			{
				modelID = ModelID::HOUSE5;
			}
			else if (House6 == filename)
			{
				modelID = ModelID::HOUSE6;
			}
			else if (House7 == filename)
			{
				modelID = ModelID::HOUSE7;
			}
			else if (House8 == filename)
			{
				modelID = ModelID::HOUSE8;
			}
			else if (House9 == filename)
			{
				modelID = ModelID::HOUSE9;
			}
			else if (House10 == filename)
			{
				modelID = ModelID::HOUSE10;
			}
			else if (HouseRoof == filename)
			{
				modelID = ModelID::HOUSEROOF;
			}
			else if (Tree3 == filename)
			{
				modelID = ModelID::TREE3;
			}
			else if (Tree5 == filename)
			{
				modelID = ModelID::TREE5;
			}
			else if (Tree6 == filename)
			{
				modelID = ModelID::TREE6;
			}
			else if (Tree8 == filename)
			{
				modelID = ModelID::TREE8;
			}

			Assimp::Importer importer;

			const aiScene* aiScene = importer.ReadFile
			(
				colliderPath,
				aiProcess_JoinIdenticalVertices |
				aiProcess_ConvertToLeftHanded
			);

			if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode)
			{
#ifdef _DEBUG
				LOG_WARNING("[Bounds] Assimp error: %s", importer.GetErrorString());
#endif 
				importer.FreeScene();
				return false;
			}

			if (!aiScene->HasMeshes())
			{
#ifdef _DEBUG
				LOG_WARNING("[Bounds] has no meshes...");
#endif 
				importer.FreeScene();
				return false;
			}
			// Go through all the meshes and create boundingboxes for them
			for (UINT i = 0; i < aiScene->mNumMeshes; i++)
			{
				const aiMesh* mesh = aiScene->mMeshes[i];

				aiNode* node = aiScene->mRootNode->FindNode(mesh->mName);

				if (node)
				{
					aiVector3D pos;
					aiVector3D scl;
					aiQuaternion rot;
					node->mTransformation.Decompose(scl, rot, pos);

					dx::XMFLOAT3 center = { pos.x, pos.y, pos.z };
					const float radius = scl.x;

					dx::BoundingSphere boS(center, scl.x);

					//Entity collider = game->GetScene("Game").CreateEntity();
					//collider.AddComponent<comp::BoundingSphere>()->Center = center;
					//collider.GetComponent<comp::BoundingSphere>()->Radius = radius;
					//collider.AddComponent<comp::Tag<TagType::STATIC>>();
					//collider.AddComponent<comp::Tag<TagType::MAP_BOUNDS>>();

					game->m_LOSColliders.push_back(std::make_pair(modelID, boS));
				}
			}
		}

		file.close();

		return true;
	}

	void LoadResources(Game* game)
	{
		std::fstream file;
		file.open(RESOURCELOADER);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to load GameScene!");
			return;
		}

		while (!file.eof())
		{
			std::string filename;

			file >> filename;

			ResourceManager::Get().GetResource<RModel>(filename);
		}

		file.close();

		file.open(TEXTUREPATH + "Loader.txt");
		if (!file.is_open())
		{
			LOG_ERROR("Failed to load textures!");
			return;
		}

		while (!file.eof())
		{
			std::string filename;

			file >> filename;

			ResourceManager::Get().GetResource<RTexture>(filename);
		}
		file.close();

		file.open(ANIMATIONPATH + "Loader.txt");

		if (!file.is_open())
		{
			LOG_ERROR("Failed to load Animations!");
			return;
		}

		while (!file.eof())
		{
			std::string filename;

			file >> filename;

			ResourceManager::Get().GetResource<RAnimation>(filename);
		}
		file.close();

		file.open(ANIMATORPATH + "Loader.txt");

		if (!file.is_open())
		{
			LOG_ERROR("Failed to load Animations!");
			return;
		}

		while (!file.eof())
		{
			std::string filename;

			file >> filename;

			ResourceManager::Get().GetResource<RAnimator>(filename);
		}
		file.close();
	}

	void LoadGameScene(Game* game)
	{
		std::fstream file;
		file.open(ASSETLOADER);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to load GameScene!");
			return;
		}

		while (!file.eof())
		{
			std::string filename;

			file >> filename;

			Entity e = game->GetScene("Game").CreateEntity();
			e.AddComponent<comp::Transform>();
			e.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>(filename);

			// Remove .obj / .fbx
			size_t count = filename.find_last_of('.');
			filename = filename.substr(0, count);

			if (House5 == filename || Door5 == filename)
			{
				game->m_models[ModelID::HOUSE5].push_back(e);
			}
			else if (House6 == filename || Door6 == filename)
			{
				game->m_models[ModelID::HOUSE6].push_back(e);
			}
			else if (House7 == filename || Door7 == filename)
			{
				game->m_models[ModelID::HOUSE7].push_back(e);
			}
			else if (House8 == filename || Door8 == filename)
			{
				game->m_models[ModelID::HOUSE8].push_back(e);
			}
			else if (House9 == filename || Door9 == filename)
			{
				game->m_models[ModelID::HOUSE9].push_back(e);
			}
			else if (House10 == filename || Door10 == filename)
			{
				game->m_models[ModelID::HOUSE10].push_back(e);
			}
			else if (HouseRoof == filename)
			{
				game->m_models[ModelID::HOUSEROOF].push_back(e);
			}
			else if (Tree3 == filename)
			{
				game->m_models[ModelID::TREE3].push_back(e);
			}
			else if (Tree5 == filename)
			{
				game->m_models[ModelID::TREE5].push_back(e);
			}
			else if (Tree6 == filename)
			{
				game->m_models[ModelID::TREE6].push_back(e);
			}
			else if (Tree8 == filename)
			{
				game->m_models[ModelID::TREE8].push_back(e);
			}
		}

		file.close();
	}

}
