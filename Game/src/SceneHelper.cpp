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
#include "LobbyUI.h"

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

		lightEntity.AddComponent<comp::SphereCollider>()->Center = sm::Vector3(pos.x, pos.y, pos.z);
		lightEntity.GetComponent<comp::SphereCollider>()->Radius = 3.f;



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

		float sunIntensity = 0.09f;
		float moonIntensity = 0.008f;
		// The sun
		Entity sun = CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { -1.0f, 0.0f, -1.f, 0.f }, { 255.f, 185, 150, 0.f }, 1000.f, sunIntensity, TypeLight::DIRECTIONAL, 1);
		// The moon
		Entity moon = CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { -1.0f, 0.0f, -1.f, 0.f }, { 50.f, 50, 200, 0.f }, 1000.f, moonIntensity, TypeLight::DIRECTIONAL, 0);

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
				Systems::SelfDestructSystem(scene, e.dt);

				Systems::RotateWatermillWheel(scene, e.dt);

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

						float d = std::abs(dir.Dot(sm::Vector3::Up));

						l = moon.GetComponent<comp::Light>();
						angle -= 180.f;
					}
				}

				Collection2D* bullColl = game->GetScene("Game").GetCollection("bullDoze");
				rtd::Picture* bullIcon = dynamic_cast<rtd::Picture*>(bullColl->elements[0].get());

				if (game->GetCycler().HasChangedPeriod())
				{
					switch (game->GetCycler().GetTimePeriod())
					{
					case CyclePeriod::NIGHT:
					{
						Collection2D* skipButtonUI = scene.GetCollection("SkipUI");
						skipButtonUI->Hide();

						game->m_players.at(game->m_localPID).GetComponent<comp::Player>()->wantsToSkipDay = false;
						game->GetScene("Game").GetCollection("shopMenu")->Hide();
						game->SetShopItem(ShopItem::None);
						bullColl->Hide();

						SoundHandler::Get().SetCurrentMusic("NightTheme");
						Collection2D* waveColl = game->GetCurrentScene()->GetCollection("ZWaveCounter");
						if (waveColl)
						{
							rtd::Text* waveText = dynamic_cast<rtd::Text*>(waveColl->elements[1].get());
							if (waveText)
							{
								waveText->SetText(std::to_string(++game->GetWaveCounter()));
							}
						}

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
						Collection2D* skipButtonUI = scene.GetCollection("SkipUI");
						skipButtonUI->Show();
						//rtd::Button* skipButton = dynamic_cast<rtd::Button*>(skipButtonUI->elements[0].get());
						//rtd::Text* skipText = dynamic_cast<rtd::Text*>(skipButtonUI->elements[1].get());
						//skipText->SetVisiblity(true);
						//skipButton->SetVisiblity(true);

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
						//// Hide skip day button.
						//Collection2D* skipButtonUI = scene.GetCollection("SkipUI");
						//rtd::Button* skipButton = dynamic_cast<rtd::Button*>(skipButtonUI->elements[0].get());
						//rtd::Text* skipText = dynamic_cast<rtd::Text*>(skipButtonUI->elements[1].get());
						//skipText->SetVisiblity(false);
						//skipButton->SetVisiblity(false);
						//scene.ForEachComponent<comp::Light>([](comp::Light& l)
						//	{
						//		if (l.lightData.type == TypeLight::POINT)
						//		{
						//			l.lightData.enabled = 1;
						//		}
						//	});

						// Hide pricetag.
						Collection2D* priceTagUI = scene.GetCollection("priceTag");
						if (priceTagUI)
						{
							priceTagUI->Hide();
						}
						break;
					}
					case CyclePeriod::DAY:
					{
						break;
					}
					default:
						break;
					}
				}

				game->GetCycler().Update(e.dt, scene);

				ShopItem shopitem = game->GetShopItem();

				if (bullColl)
				{
					rtd::Picture* bullIcon = dynamic_cast<rtd::Picture*>(bullColl->elements[0].get());
					ShopItem shopitem = game->GetShopItem();

					if (shopitem == ShopItem::Destroy_Tool)
					{
						bullColl->Show();
						bullIcon->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
					}

					ElementState shopMenuState = game->GetScene("Game").GetCollection("shopMenu")->GetState();
					Collection2D* menu = game->GetScene("Game").GetCollection("inGameMenu");
					ElementState shopIconState = game->GetScene("Game").GetCollection("shopIcon")->GetState();

					if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::Escape, KeyState::PRESSED))
					{
						if (shopMenuState == ElementState::NONE && menu->GetState() == ElementState::NONE)
						{
							menu->Show();
						}
						else
						{
							menu->Hide();
						}
					}

					if (shopMenuState == ElementState::OUTSIDE && shopIconState == ElementState::OUTSIDE)
					{
						if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::Escape, KeyState::PRESSED) ||
							InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED) ||
							(InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED) && game->GetShopItem() == ShopItem::None))
						{
							game->GetScene("Game").GetCollection("shopMenu")->Hide();
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
		mainMenu->AddElement<rtd::Picture>("MenuBG.png", draw_t(0, 0, width, height));
		sm::Vector2 textSize = { (widthScale * 0.35f) * 1.5f, (height * 0.2f) * 1.5f };
		sm::Vector2 textPos = { (width / 2) - textSize.x / 2, (height / 2) - textSize.y / 2 };
		mainMenu->AddElement<rtd::Picture>("MainMenuText.png", draw_t(textPos.x, textPos.y - height / 8.f, textSize.x, textSize.y));
		scene.Add2DCollection(mainMenu, "AMainMenu");
		/*---------Background---------*/


		/*---------Textfields---------*/
		Collection2D* connectFields = new Collection2D;
		rtd::TextField* ipField = connectFields->AddElement<rtd::TextField>(draw_text_t((width / 2.f) - (widthScale * 0.25f), height * 0.55f, widthScale * 0.25f, D2D1Core::GetDefaultFontSize()), 25);
		ipField->SetDescriptionText("IP Address:");
		ipField->SetShape(Shapes::RECTANGLE_ROUNDED);
		padding = (widthScale / 64.f);
		rtd::TextField* portField = connectFields->AddElement<rtd::TextField>(draw_text_t((width / 2.f) + padding, height * 0.55f, widthScale * 0.25f, D2D1Core::GetDefaultFontSize()), 6);
		portField->SetDescriptionText("Port:");
		portField->SetShape(Shapes::RECTANGLE_ROUNDED);
		/*---------Textfields---------*/


		/*---------Buttons---------*/
		sm::Vector2 buttonSize = { widthScale / 6.f, height / 10.f };
		sm::Vector2 buttonPos = { (width / 2.f) - (buttonSize.x / 2.f),  height - (height * 0.37f) };
		padding = buttonSize.y + height * 0.02f;
		rtd::Button* connectButton = connectFields->AddElement<rtd::Button>("Start.png", draw_t(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y));
		buttonPos.y += padding;
		rtd::Button* settingsButton = connectFields->AddElement<rtd::Button>("Settings.png", draw_t(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y));
		buttonPos.y += padding;
		rtd::Button* quitButton = connectFields->AddElement<rtd::Button>("Quit.png", draw_t(buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y)); //width / 4.f, height * 0.15f
		scene.Add2DCollection(connectFields, "ConnectFields");

		rtd::Button* externalLinkBtn = connectFields->AddElement<rtd::Button>("Button.png", draw_t(width - width / 4.f, height - (height / 5.f), width / 8.f, height / 16.f));
		externalLinkBtn->GetText()->SetScale(0.5f);
		externalLinkBtn->GetText()->SetText("Give Feedback!");
		externalLinkBtn->SetOnPressedEvent([] {
			ShellExecuteA(NULL, "open", "https://forms.gle/1E4f4a9jqKoNpCgZ7", NULL, NULL, SW_SHOWNORMAL);
			});

		rtd::Text* deadServerText = connectFields->AddElement<rtd::Text>("Error connecting to server", draw_text_t((width / 8.0f), (height / 8.0f) * 5.0f, width / 4.0f, height / 8.0f));
		deadServerText->SetVisiblity(false);

		ipField->SetPresetText("127.0.0.1");
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
		Scene& scene = game->GetScene("Game");
		float width = (float)game->GetWindow()->GetWidth();
		float height = (float)game->GetWindow()->GetHeight();

		float widthScale = height * (16.f / 9.f);
		sm::Vector2 padding = { widthScale / 64.f , widthScale / 64.f };

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
			if (i != 0)
			{
				playerHp->Hide();
			}
			scene.Add2DCollection(playerHp, "Aplayer" + std::to_string(i + 1) + "Info");
			Collection2D* nameCollection = new Collection2D;
			nameCollection->AddElement<rtd::Text>("Player", draw_text_t(0, 0, widthScale / 14, height / 6));
			scene.Add2DCollection(nameCollection, "AdynamicPlayer" + std::to_string(i + 1) + "namePlate");
			nameCollection->Hide();
		}
		for (int i = 0; i < NR_OF_HOUSES; i++)
		{
			Collection2D* houseWarning = new Collection2D;
			houseWarning->AddElement<rtd::Picture>("WarningIcon.png", draw_t(0.f, 0.f, 40, 100));
			scene.Add2DCollection(houseWarning, "HouseWarningIcon" + std::to_string(i + 1));
			houseWarning->Hide();
		}
		sm::Vector2 moneyScale = { widthScale / 8.0f, height / 11.0f };
		sm::Vector2 moneyPos = { width - (moneyScale.x + padding.x), padding.y };
		Collection2D* money = new Collection2D;
		rtd::MoneyUI* mMoney = money->AddElement<rtd::MoneyUI>(draw_text_t(moneyPos.x, moneyPos.y, moneyScale.x, moneyScale.y));
		scene.Add2DCollection(money, "ZMoneyUI");

		Collection2D* waveAmount = new Collection2D;
		waveAmount->AddElement<rtd::Picture>("Wave.png", draw_t(moneyPos.x - (moneyScale.x) * 0.50f - padding.x, padding.y, widthScale / 16.0f, height / 22.0f));
		waveAmount->AddElement<rtd::Text>("0", draw_t(moneyPos.x - (moneyScale.x) * 0.50f - padding.x, padding.y, widthScale / 16.0f, height / 22.0f));
		scene.Add2DCollection(waveAmount, "ZWaveCounter");

		Collection2D* killAmount = new Collection2D;
		killAmount->AddElement<rtd::Picture>("Kills.png", draw_t(moneyPos.x - (moneyScale.x) * 1.50f - padding.x * 3.0f, padding.y, widthScale / 16.0f, height / 22.0f));
		killAmount->AddElement<rtd::Text>("0", draw_t(moneyPos.x - (moneyScale.x) * 1.50f - padding.x * 3.0f, padding.y, widthScale / 16.0f, height / 22.0f));
		scene.Add2DCollection(killAmount, "ZKillCounter");

		Collection2D* deathAmount = new Collection2D;
		deathAmount->AddElement<rtd::Picture>("Deaths.png", draw_t(moneyPos.x - (moneyScale.x) * 1.00f - padding.x * 2.0f, padding.y, widthScale / 16.0f, height / 22.0f));
		deathAmount->AddElement<rtd::Text>("0", draw_t(moneyPos.x - (moneyScale.x) * 1.00f - padding.x * 2.0f, padding.y, widthScale / 16.0f, height / 22.0f));
		scene.Add2DCollection(deathAmount, "ZDeathCounter");



		Collection2D* abilities = new Collection2D;
		sm::Vector2 barPos = { ((width / 2.f)) - ((widthScale / 16.0f) * 2.5f), height - (height / 9.0f + padding.y) };
		rtd::Picture* abilityBar = abilities->AddElement<rtd::Picture>("AbilityBar.png", draw_t(barPos.x, barPos.y, (widthScale / 16.0f) * 5.0f, height / 9.0f));

		sm::Vector2 abillitySize = { widthScale / 18.0f, height / 11.0f };
		sm::Vector2 abillityPos = { (width / 2.f) - (abillitySize.x / 2), barPos.y + (padding.y * 0.25f) };

		rtd::AbilityUI* primary = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x - (abillitySize.x * 2 + padding.x), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "Attack2.png");
		primary->SetActivateButton("LMB");
		primary->SetReference(&game->m_primaryCooldown);
		primary->SetMaxReference(&game->m_primaryMaxCooldown);

		rtd::AbilityUI* secondary = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x - (abillitySize.x + padding.x / 2), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "Block.png");
		secondary->SetActivateButton("RMB");
		secondary->SetReference(&game->m_secondaryCooldown);
		secondary->SetMaxReference(&game->m_secondaryMaxCooldown);

		rtd::AbilityUI* third = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x, abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "Dodge.png");
		third->SetActivateButton("Shift");
		third->SetReference(&game->m_dodgeCooldown);
		third->SetMaxReference(&game->m_dodgeMaxCooldown);

		rtd::AbilityUI* fourth = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x + (abillitySize.x + padding.x / 2), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "LockedIcon.png");
		rtd::AbilityUI* fith = abilities->AddElement<rtd::AbilityUI>(draw_t(abillityPos.x + (abillitySize.x * 2 + padding.x), abillityPos.y, abillitySize.x, abillitySize.y), D2D1::ColorF(0, 1.0f), "LockedIcon.png");
		scene.Add2DCollection(abilities, "ZAbilityUI");

		Collection2D* spectatingCollection = new Collection2D;
		rtd::Text* deadText = spectatingCollection->AddElement<rtd::Text>("You are dead!", draw_t((width / 2) - (widthScale / 17.f), (height / 2) + (height / 8.5f), (widthScale / 10.f), (height / 8.f)));
		rtd::Text* spectateText = spectatingCollection->AddElement<rtd::Text>("LMB to spectate another player", draw_t((width / 2) - (widthScale / 6.f), (height / 4.f) + (height / 2.5f), (widthScale / 3.f), (height / 8.f)));
		scene.Add2DCollection(spectatingCollection, "SpectateUI");
		deadText->SetVisiblity(false);
		spectateText->SetVisiblity(false);
		
		Collection2D* skipCollection = new Collection2D;
		rtd::Button* skipToNightButton = skipCollection->AddElement<rtd::Button>("Button.png", draw_t(width - ((widthScale / 9.f)), (height / 2) - (height / 8), (widthScale / 10.f), (height / 8.f)), false);
		rtd::Text* skipToNightText = skipCollection->AddElement<rtd::Text>("Skip Day", draw_t(width - ((widthScale / 9.f)), (height / 2) - (height / 8), (widthScale / 10.f), (height / 8.f)));

		scene.Add2DCollection(skipCollection, "SkipUI");
		skipToNightButton->SetOnPressedEvent([=]
			{
				Entity player;
				game->GetLocalPlayer(player);
				game->SetPlayerWantsToSkip(true);
				skipCollection->Hide();
				//skipCollection->elements[0].get()->SetVisiblity(false);
				//skipCollection->elements[1].get()->SetVisiblity(false);
			});
		skipToNightButton->SetOnHoverEvent([=]()
			{
				game->m_inputState.leftMouse = false;
				game->m_inputState.rightMouse = false;
			});

		Collection2D* pauseMenu = new Collection2D;
		sm::Vector2 pauseMenuPos = { width * 0.5f - (widthScale * 0.125f), (height / 2) - (height * 0.25f) };
		sm::Vector2 pauseMenuScale = { widthScale * 0.25f, height * 0.5f };
		rtd::Picture* inGameMenu = pauseMenu->AddElement<rtd::Picture>("MenuBackground.png", draw_t(pauseMenuPos.x, pauseMenuPos.y, pauseMenuScale.x, pauseMenuScale.y));
		sm::Vector2 buttonPosPauseMenu = { pauseMenuPos.x + padding.x * 3 , pauseMenuPos.y + padding.y * 1.5f };
		sm::Vector2 buttonScalePauseMenu = { pauseMenuScale.x - padding.x * 6.f, (pauseMenuScale.x - padding.x * 6.f) / 2.4723618090f };
		rtd::Button* resumeButton = pauseMenu->AddElement<rtd::Button>("Resume.png", draw_t(buttonPosPauseMenu.x, buttonPosPauseMenu.y, buttonScalePauseMenu.x, buttonScalePauseMenu.y));
		buttonPosPauseMenu.y += buttonScalePauseMenu.y + padding.y;
		rtd::Button* settingsButton = pauseMenu->AddElement<rtd::Button>("Settings.png", draw_t(buttonPosPauseMenu.x, buttonPosPauseMenu.y, buttonScalePauseMenu.x, buttonScalePauseMenu.y));
		buttonPosPauseMenu.y += buttonScalePauseMenu.y + padding.y;
		rtd::Button* quitButton = pauseMenu->AddElement<rtd::Button>("Quit.png", draw_t(buttonPosPauseMenu.x, buttonPosPauseMenu.y, buttonScalePauseMenu.x, buttonScalePauseMenu.y));

		inGameMenu->SetOnHoverEvent([=]()
			{
				game->m_inputState.leftMouse = 0;
				game->m_inputState.rightMouse = 0;
			});
		quitButton->SetOnPressedEvent([=]
			{
				game->Shutdown();
			});
		quitButton->SetOnHoverEvent([=]()
			{
				game->m_inputState.leftMouse = 0;
				game->m_inputState.rightMouse = 0;
			});
		settingsButton->SetOnPressedEvent([=]
			{
				pauseMenu->Hide();
			});
		settingsButton->SetOnHoverEvent([=]()
			{
				game->m_inputState.leftMouse = 0;
				game->m_inputState.rightMouse = 0;
			});
		resumeButton->SetOnPressedEvent([=]
			{
				pauseMenu->Hide();
			});
		resumeButton->SetOnHoverEvent([=]()
			{
				game->m_inputState.leftMouse = 0;
				game->m_inputState.rightMouse = 0;
			});

		scene.Add2DCollection(pauseMenu, "inGameMenu");
		pauseMenu->Hide();

		Collection2D* shopIcon = new Collection2D;
		Collection2D* shopMenu = new Collection2D;
		rtd::Button* button;

		button = shopIcon->AddElement<rtd::Button>("ShopIcon.png", draw_t(0.0f, 0.0f, widthScale / 24, height / 14));
		button->SetOnPressedEvent([=]()
			{
				if (game->GetCycler().GetTimePeriod() != CyclePeriod::NIGHT)
				{
					shopMenu->Show();
					bullDoze->Hide();
				}
			});
		button->SetOnHoverEvent([=]()
			{
				game->m_inputState.leftMouse = false;
				game->m_inputState.rightMouse = false;
			});

		scene.Add2DCollection(shopIcon, "shopIcon");

		rtd::ShopUI* shop = shopMenu->AddElement<rtd::ShopUI>("Shop.png", draw_t(width / 24.0f, (height / 16), widthScale * 0.25f, height * 0.5f));
		shop->SetOnHoverEvent([=]()
			{
				game->m_inputState.leftMouse = false;
				game->m_inputState.rightMouse = false;
			});
		// 1x1 tower button.
		shop->SetOnPressedEvent(0, [=]()
			{
				game->SetShopItem(ShopItem::Defence1x1);
				audio_t audio = {};
				audio.isUnique = false;
				audio.volume = SoundHandler::Get().GetMasterVolume();
				SoundHandler::Get().PlaySound("ButtonClick", audio);
				bullDoze->Hide();
			});
		// 1x3 tower button.
		shop->SetOnPressedEvent(1, [=]()
			{
				game->SetShopItem(ShopItem::Defence1x3);
				audio_t audio = {};
				audio.isUnique = false;
				audio.volume = SoundHandler::Get().GetMasterVolume();
				SoundHandler::Get().PlaySound("ButtonClick", audio);
				bullDoze->Hide();
			});
		// Primary upgrade button.
		shop->SetOnPressedEvent(2, [=]()
			{
				game->SetShopItem(ShopItem::Primary_Upgrade);
				audio_t audio = {};
				audio.isUnique = false;
				audio.volume = SoundHandler::Get().GetMasterVolume();
				SoundHandler::Get().PlaySound("ButtonClick", audio);
				bullDoze->Hide();
			});
		// Armor upgrade button.
		shop->SetOnPressedEvent(3, [=]()
			{
				game->SetShopItem(ShopItem::Primary_Upgrade);
				audio_t audio = {};
				audio.isUnique = false;
				audio.volume = SoundHandler::Get().GetMasterVolume();
				SoundHandler::Get().PlaySound("ButtonClick", audio);
				bullDoze->Hide();
			});
		// Heal button.
		shop->SetOnPressedEvent(4, [=]()
			{
				game->SetShopItem(ShopItem::Heal);
				audio_t audio = {};
				audio.isUnique = false;
				audio.volume = SoundHandler::Get().GetMasterVolume();
				SoundHandler::Get().PlaySound("ButtonClick", audio);
				bullDoze->Hide();
			});
		// Remove defences button.
		shop->SetOnPressedEvent(5, [=]()
			{
				game->SetShopItem(ShopItem::Destroy_Tool);
				audio_t audio = {};
				audio.isUnique = false;
				audio.volume = SoundHandler::Get().GetMasterVolume();
				SoundHandler::Get().PlaySound("ButtonClick", audio);
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
		const float paddingHeight = paddingWidth;

		Scene& scene = game->GetScene("Lobby");

		sm::Vector2 scale = { widthScale / 4, height / 9 };
		sm::Vector2 lobbyPos = { width / 16.f, scale.y + paddingHeight };
		sm::Vector2 classIconPos = { lobbyPos.x + scale.x + paddingWidth, lobbyPos.y };

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			Collection2D* playerIcon = new Collection2D;
			playerIcon->AddElement<rtd::Picture>("Button.png", draw_t(lobbyPos.x, lobbyPos.y, scale.x, scale.y));
			playerIcon->AddElement<rtd::Text>("Player " + std::to_string(i + 1), draw_text_t(lobbyPos.x, lobbyPos.y, scale.x, scale.y));
			playerIcon->AddElement<rtd::Picture>("WarriorIcon.png", draw_t(classIconPos.x, classIconPos.y, scale.x / 4, scale.y));
			playerIcon->AddElement<rtd::Picture>("Yes.png", draw_t(classIconPos.x + paddingWidth + (scale.x / 4), lobbyPos.y, scale.x / 4, scale.y))->SetVisiblity(false);
			scene.Add2DCollection(playerIcon, "playerIcon" + std::to_string(i + 1));

			lobbyPos.y += scale.y + paddingHeight;
			classIconPos.y = lobbyPos.y;
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
		float widthScale = height * (16.f / 9.f);
		sm::Vector2 padding = { widthScale / 64.f, widthScale / 64.f };

		sm::Vector2 scaleCat = { widthScale / 6.f, height / 9.f };
		sm::Vector2 scale = { scaleCat.x * 0.57f, scaleCat.y * 0.57f };
		Scene& scene = game->GetScene("Options");

		/*---------General---------*/
		Collection2D* general = new Collection2D;

		general->AddElement<rtd::Picture>("MenuBG.png", draw_t(0, 0, width, height));
		rtd::Button* exitButton = general->AddElement<rtd::Button>("No.png", draw_t(padding.x, padding.y, widthScale / 24, height / 14), false);
		exitButton->SetOnPressedEvent([=]()
			{
				game->SetScene("MainMenu");
				OptionSystem::Get().SetOption("BloomIntensity", std::to_string(thread::RenderThreadHandler::Get().GetRenderer()->GetBloomPass()->AdjustBloomIntensity()));
				OptionSystem::Get().OnShutdown();
			});
		scene.Add2DCollection(general, "AMenuBG");
		/*---------General---------*/

		/*---------Categories---------*/
		Collection2D* category = new Collection2D;
		sm::Vector2 posCat = { padding.x, ((height / 2.f) - scaleCat.y / 2.f) - (padding.y + scaleCat.y) };
		sm::Vector2 canvasSize = { width - posCat.x - scaleCat.x - padding.x * 2.f, height - padding.y * 2 };
		sm::Vector2 canvasPos = { posCat.x + scaleCat.x + padding.x, height - (canvasSize.y + padding.y) };

		rtd::Button* videoCat = category->AddElement<rtd::Button>("Button.png", draw_t(posCat.x, posCat.y, scaleCat.x, scaleCat.y));
		category->AddElement<rtd::Text>("Video", draw_t(posCat.x, posCat.y, scaleCat.x, scaleCat.y));
		posCat.y += padding.y + scaleCat.y;
		rtd::Button* audioCat = category->AddElement<rtd::Button>("Button.png", draw_t(posCat.x, posCat.y, scaleCat.x, scaleCat.y));
		category->AddElement<rtd::Text>("Audio", draw_t(posCat.x, posCat.y, scaleCat.x, scaleCat.y));
		posCat.y += padding.y + scaleCat.y;
		rtd::Button* graphicsCat = category->AddElement<rtd::Button>("Button.png", draw_t(posCat.x, posCat.y, scaleCat.x, scaleCat.y));
		category->AddElement<rtd::Text>("Graphics", draw_t(posCat.x, posCat.y, scaleCat.x, scaleCat.y));

		category->AddElement<rtd::Canvas>(D2D1::ColorF(178.f / 255.f, 44.f / 255.f, 65.f / 255.f), draw_t(canvasPos.x, canvasPos.y, canvasSize.x, canvasSize.y));
		scene.Add2DCollection(category, "Category");
		/*---------Categories---------*/


		/*---------Video---------*/
		Collection2D* videoCategory = new Collection2D;
		sm::Vector2 posVideo = { width - (scale.x + padding.x * 2), canvasPos.y + padding.y };

		videoCategory->AddElement<rtd::Text>("Fullscreen", draw_t(canvasPos.x + padding.x, posVideo.y, scale.x, scale.y));
		rtd::Border* fullscreenBorder = videoCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, posVideo.y, canvasSize.x - padding.x * 2, scale.y));
		fullscreenBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		fullscreenBorder->SetLineWidth(LineWidth::LARGE);
		rtd::Button* fullscreenOpt = videoCategory->AddElement<rtd::Button>("Button.png", draw_t(posVideo.x, posVideo.y, scale.x, scale.y));
		rtd::Text* fullscreenButtonText = videoCategory->AddElement<rtd::Text>("Yes", draw_t(posVideo.x, posVideo.y, scale.x, scale.y));

		posVideo.y += scale.y + padding.y * 1.5f;
		videoCategory->AddElement<rtd::Text>("Resolution if not fullscreen", draw_t(canvasPos.x - padding.x, posVideo.y, scale.x * 3.f, scale.y));
		rtd::Border* resolutionBorder = videoCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, posVideo.y, canvasSize.x - padding.x * 2, scale.y));
		resolutionBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		resolutionBorder->SetLineWidth(LineWidth::LARGE);
		rtd::Button* resolutionOpt = videoCategory->AddElement<rtd::Button>("Button.png", draw_t(posVideo.x, posVideo.y, scale.x, scale.y));
		rtd::Text* resButtonText = videoCategory->AddElement<rtd::Text>("1920x1080", draw_t(posVideo.x, posVideo.y, scale.x, scale.y));

		//Resolution Button
		{
			static int winWidth = std::stoi(OptionSystem::Get().GetOption("WindowWidth"));
			static int winHeight = std::stoi(OptionSystem::Get().GetOption("WindowHeight"));

			switch (winWidth)
			{
			case 1920:
			{
				resButtonText->SetText("1920x1080");
				break;
			}
			case 2560:
			{
				resButtonText->SetText("2560x1440");
				break;
			}
			case 1408:
			{
				resButtonText->SetText("1408x792");
				break;
			}
			case 1536:
			{
				resButtonText->SetText("1536x864");
				break;
			}
			case 1632:
			{
				resButtonText->SetText("1632x918");
				break;
			}
			default:
			{
				winWidth = 1920;
				winHeight = 1080;
				break;
			}
			};
			resolutionOpt->SetOnPressedEvent([=] {

				switch (winWidth)
				{
				case 1920:
				{
					resButtonText->SetText("2560x1440");
					winWidth = 2560;
					winHeight = 1440;
					break;
				}
				case 2560:
				{
					resButtonText->SetText("1408x792");
					winWidth = 1408;
					winHeight = 792;
					break;
				}
				case 1408:
				{
					resButtonText->SetText("1536x864");
					winWidth = 1536;
					winHeight = 864;
					break;
				}
				case 1536:
				{
					resButtonText->SetText("1632x918");
					winWidth = 1632;
					winHeight = 918;
					break;
				}
				case 1632:
				{
					resButtonText->SetText("1920x1080");
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
		}

		//Fullscreen button
		{
			static int fullscreen = std::stoi(OptionSystem::Get().GetOption("Fullscreen"));
			if (fullscreen == 0)
			{
				fullscreenButtonText->SetText("No");
			}
			else
			{
				fullscreenButtonText->SetText("Yes");
			}

			fullscreenOpt->SetOnPressedEvent([=]
				{
					if (fullscreen == 0)
					{
						fullscreenButtonText->SetText("Yes");
						fullscreen = 1;
					}
					else
					{
						fullscreenButtonText->SetText("No");
						fullscreen = 0;
					}

					OptionSystem::Get().SetOption("Fullscreen", std::to_string(fullscreen));
				});
		}

		scene.Add2DCollection(videoCategory, "WVideo");
		/*---------Video---------*/

		/*---------Audio---------*/
		Collection2D* audioCategory = new Collection2D;
		sm::Vector2 posAudio = { width - (scale.x + padding.x * 2), canvasPos.y + padding.y };

		sm::Vector2 minPos = { canvasPos.x + padding.x + canvasSize.x / 2.f, posAudio.y };
		sm::Vector2 maxPos = { width - (scale.x + padding.x * 2.f), posAudio.y };
		rtd::Canvas* masterVolCanvas = audioCategory->AddElement<rtd::Canvas>(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f), draw_t(minPos.x - 6.f, posAudio.y, ((canvasSize.x / 2.f) + 6.f) - padding.x * 2.f, scale.y));
		masterVolCanvas->SetShape(Shapes::RECTANGLE_ROUNDED);
		rtd::Slider* masterVolumeSL = audioCategory->AddElement<rtd::Slider>(D2D1::ColorF(0, 0, 0), draw_t(minPos.x, posAudio.y, scale.x, scale.y), &SoundHandler::Get().AdjustMasterVolume(), 1.0f, 0.0f);
		masterVolumeSL->SetMinPos(minPos);
		masterVolumeSL->SetMaxPos(maxPos);
		rtd::Text* valueText = masterVolumeSL->GetValueText();
		sm::Vector2 valueTextPos = { minPos.x + (maxPos.x - minPos.x) / 2 + (valueText->GetText().length() * D2D1Core::GetDefaultFontSize()) / 2.f, posAudio.y };
		valueText->SetPosition(valueTextPos.x, valueTextPos.y);

		audioCategory->AddElement<rtd::Text>("Master", draw_t(canvasPos.x + padding.x, posAudio.y, scale.x, scale.y));
		rtd::Border* lineBorder = audioCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, posAudio.y, canvasSize.x - padding.x * 2.f, scale.y));
		lineBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		lineBorder->SetLineWidth(LineWidth::LARGE);

		scene.Add2DCollection(audioCategory, "WAudio");
		/*---------Audio---------*/

		/*---------Graphics---------*/
		Collection2D* graphicsCategory = new Collection2D;
		sm::Vector2 graphicsPos = { width - (scale.x + padding.x * 2), canvasPos.y + padding.y };

		//Depth of Field
		rtd::Border* dofBorder = graphicsCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, graphicsPos.y, canvasSize.x - padding.x * 2, scale.y));
		dofBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		dofBorder->SetLineWidth(LineWidth::LARGE);
		graphicsCategory->AddElement<rtd::Text>("Depth of Field", draw_t(canvasPos.x + padding.x, graphicsPos.y, scale.x + padding.x, scale.y));
		rtd::Button* dofOpt = graphicsCategory->AddElement<rtd::Button>("Button.png", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));
		rtd::Text* dofText = graphicsCategory->AddElement<rtd::Text>("None", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));

		//DoF Button Events
		{
			static DoFType type = DoFType::DEFAULT;
			std::string savedBlur = OptionSystem::Get().GetOption("BlurType");
			type = static_cast<DoFType>(std::stod(savedBlur));
			// Toggle when the game starts.
			switch (type)
			{
			case DoFType::VIGNETTE:
			{
				dofText->SetText("Static");
				break;
			}
			case DoFType::DEFAULT:
			{
				dofText->SetText("None");
				break;
			}
			case DoFType::STATIC:
			{
				dofText->SetText("Adaptive");
				break;
			}
			default:
			{
				type = DoFType::DEFAULT;
				break;
			}
			}
			OptionSystem::Get().SetOption("BlurType", std::to_string(static_cast<int>(type)));

			dofOpt->SetOnPressedEvent([=] {
				// Toggle between types.
				switch (type)
				{
				case DoFType::STATIC:
				{
					type = DoFType::VIGNETTE;
					dofText->SetText("Static");
					thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::VIGNETTE);
					OptionSystem::Get().SetOption("BlurType", std::string("2"));
					break;
				}
				case DoFType::VIGNETTE:
				{
					type = DoFType::DEFAULT;
					dofText->SetText("None");
					thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::DEFAULT);
					OptionSystem::Get().SetOption("BlurType", std::string("0"));
					break;
				}
				case DoFType::DEFAULT:
				{
					type = DoFType::STATIC;
					dofText->SetText("Adaptive");
					thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::STATIC);
					OptionSystem::Get().SetOption("BlurType", std::string("1"));
					break;
				}
				default:
					break;
				}

				});
		}

		//Volumetric Lighting
		graphicsPos.y += scale.y + padding.y;
		rtd::Border* volumetricBorder = graphicsCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, graphicsPos.y, canvasSize.x - padding.x * 2, scale.y));
		volumetricBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		volumetricBorder->SetLineWidth(LineWidth::LARGE);
		graphicsCategory->AddElement<rtd::Text>("Volumetric Lighting", draw_t(canvasPos.x, graphicsPos.y, scale.x * 2.f, scale.y));
		rtd::Button* volumetricOpt = graphicsCategory->AddElement<rtd::Button>("Button.png", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));
		rtd::Text* volumetricText = graphicsCategory->AddElement<rtd::Text>("Medium", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));

		//Volumetric Button Events
		{
			static int lightQuality = std::stoi(OptionSystem::Get().GetOption("VolumetricLightQuality"));
			if (lightQuality <= 15 && lightQuality > 0)
			{
				volumetricText->SetText("LOW");
			}
			else if (lightQuality > 15 && lightQuality <= 50)
			{
				volumetricText->SetText("MEDIUM");
			}
			else if (lightQuality > 50 && lightQuality <= 100)
			{
				volumetricText->SetText("HIGH");
			}
			else if (lightQuality > 100 && lightQuality <= 200)
			{
				volumetricText->SetText("INSANE");
			}
			else
			{
				volumetricText->SetText("MEDIUM");
				lightQuality = 50;
			}

			game->GetScene("Game").GetLights()->SetLightVolumeQuality(lightQuality);
			OptionSystem::Get().SetOption("VolumetricLightQuality", std::to_string(lightQuality));

			volumetricOpt->SetOnPressedEvent([=] {

				switch (lightQuality)
				{
				case 15:
				{
					lightQuality = 50;
					volumetricText->SetText("MEDIUM");
					break;
				}
				case 50:
				{
					lightQuality = 100;
					volumetricText->SetText("HIGH");
					break;
				}
				case 100:
				{
					lightQuality = 200;
					volumetricText->SetText("INSANE");
					break;
				}
				case 200:
				{
					lightQuality = 15;
					volumetricText->SetText("LOW");
					break;
				}
				default:
				{
					lightQuality = 15;
					volumetricText->SetText("LOW");
					break;
				}
				};

				OptionSystem::Get().SetOption("VolumetricLightQuality", std::to_string(lightQuality));
				game->GetScene("Game").GetLights()->SetLightVolumeQuality(lightQuality);

				});
		}

		//Bloom
		graphicsPos.y += scale.y + padding.y;
		rtd::Border* bloomBorder = graphicsCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, graphicsPos.y, canvasSize.x - padding.x * 2, scale.y));
		bloomBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		bloomBorder->SetLineWidth(LineWidth::LARGE);
		graphicsCategory->AddElement<rtd::Text>("Bloom Effect", draw_t(canvasPos.x - padding.x, graphicsPos.y, scale.x * 2.f, scale.y));
		rtd::Button* bloomOpt = graphicsCategory->AddElement<rtd::Button>("Button.png", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));
		rtd::Text* bloomText = graphicsCategory->AddElement<rtd::Text>("ON", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));
		static std::string bloomOption = OptionSystem::Get().GetOption("Bloom");
		if (bloomOption == "0")
			bloomOption = "ON";

		if (bloomOption == "ON")
		{
			bloomText->SetText("ON");
			thread::RenderThreadHandler::Get().GetRenderer()->GetBloomPass()->SetEnable(true);
		}
		else // OFF
		{
			bloomText->SetText("OFF");
			thread::RenderThreadHandler::Get().GetRenderer()->GetBloomPass()->SetEnable(false);
		}
		OptionSystem::Get().SetOption("Bloom", bloomOption);

		bloomOpt->SetOnPressedEvent([=] {

			if (bloomOption == "ON")
			{
				bloomText->SetText("OFF");
				bloomOption = "OFF";
				thread::RenderThreadHandler::Get().GetRenderer()->GetBloomPass()->SetEnable(false);
			}
			else if (bloomOption == "OFF")
			{

				bloomText->SetText("ON");
				bloomOption = "ON";
				thread::RenderThreadHandler::Get().GetRenderer()->GetBloomPass()->SetEnable(true);
			}

			OptionSystem::Get().SetOption("Bloom", bloomOption);

			});

		// Bloom Intensity
		graphicsPos.y += scale.y + padding.y;
		rtd::Border* bloomIntensityBorder = graphicsCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, graphicsPos.y, canvasSize.x - padding.x * 2, scale.y));
		bloomIntensityBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		bloomIntensityBorder->SetLineWidth(LineWidth::LARGE);
		rtd::Canvas* bloomVolCanvas = graphicsCategory->AddElement<rtd::Canvas>(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f), draw_t(minPos.x - 6.f, graphicsPos.y, ((canvasSize.x / 2.f) + 6.f) - padding.x * 2.f, scale.y));
		bloomVolCanvas->SetShape(Shapes::RECTANGLE_ROUNDED);
		graphicsCategory->AddElement<rtd::Text>("Bloom Intensity", draw_t(canvasPos.x - padding.x, graphicsPos.y, scale.x * 2.f, scale.y));
		rtd::Slider* bloomVolumeSL = graphicsCategory->AddElement<rtd::Slider>(D2D1::ColorF(0, 0, 0), draw_t(minPos.x, graphicsPos.y, scale.x, scale.y), &thread::RenderThreadHandler::Get().GetRenderer()->GetBloomPass()->AdjustBloomIntensity(), 1.0f, 0.05f);
		bloomVolumeSL->SetMinPos(minPos);
		bloomVolumeSL->SetMaxPos(maxPos);
		rtd::Text* bloomValueText = bloomVolumeSL->GetValueText();
		sm::Vector2 bloomValueTextPos = { minPos.x + (maxPos.x - minPos.x) / 2 + (bloomValueText->GetText().length() * D2D1Core::GetDefaultFontSize()) / 2.f, graphicsPos.y };
		bloomValueText->SetPosition(bloomValueTextPos.x, bloomValueTextPos.y);

		//Shadows
		graphicsPos.y += scale.y + padding.y;
		rtd::Border* shadowsBorder = graphicsCategory->AddElement<rtd::Border>(draw_t(canvasPos.x + padding.x, graphicsPos.y, canvasSize.x - padding.x * 2, scale.y));
		shadowsBorder->SetColor(D2D1::ColorF(53.f / 255.f, 22.f / 255.f, 26.f / 255.f));
		shadowsBorder->SetLineWidth(LineWidth::LARGE);
		graphicsCategory->AddElement<rtd::Text>("Shadow Quality", draw_t(canvasPos.x - padding.x, graphicsPos.y, scale.x * 2.f, scale.y));
		rtd::Button* shadowOpt = graphicsCategory->AddElement<rtd::Button>("Button.png", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));
		rtd::Text* shadowText = graphicsCategory->AddElement<rtd::Text>("Medium", draw_t(graphicsPos.x, graphicsPos.y, scale.x, scale.y));

		//Shadow Button Events
		{
			static std::string shadowQuality = OptionSystem::Get().GetOption("ShadowQuality");
			if (shadowQuality == "Potato")
			{
				shadowText->SetText("Potato");
			}
			else if (shadowQuality == "Low")
			{
				shadowText->SetText("Low");
			}
			else if (shadowQuality == "Medium")
			{
				shadowText->SetText("Medium");
			}
			else if (shadowQuality == "High")
			{
				shadowText->SetText("High");
			}
			else if (shadowQuality == "Insane")
			{
				shadowText->SetText("Insane");
			}
			else
			{
				shadowText->SetText("Shadows Quality: Medium");
				shadowQuality = "Medium";
				OptionSystem::Get().SetOption("ShadowQuality", std::string("Medium"));
			}

			shadowOpt->SetOnPressedEvent([=] {

				if (shadowQuality == "Potato")
				{
					shadowText->SetText("Low");
					shadowQuality = "Low";
					thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(256);
				}
				else if (shadowQuality == "Low")
				{
					shadowText->SetText("Medium");
					shadowQuality = "Medium";
					thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(1024);
				}
				else if (shadowQuality == "Medium")
				{
					shadowText->SetText("High");
					shadowQuality = "High";
					thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(2048);
				}
				else if (shadowQuality == "High")
				{
					shadowText->SetText("Insane");
					shadowQuality = "Insane";
					thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(4096);
				}
				else if (shadowQuality == "Insane")
				{
					shadowText->SetText("Potato");
					shadowQuality = "Potato";
					thread::RenderThreadHandler::Get().GetRenderer()->GetShadowPass()->SetShadowMapSize(512);
				}
				OptionSystem::Get().SetOption("ShadowQuality", shadowQuality);

				});
		}

		scene.Add2DCollection(graphicsCategory, "WGraphics");
		/*---------Graphics---------*/

		videoCategory->Show();
		audioCategory->Hide();
		graphicsCategory->Hide();

		videoCat->SetOnPressedEvent([=]
			{
				videoCategory->Show();
				audioCategory->Hide();
				graphicsCategory->Hide();
			});
		audioCat->SetOnPressedEvent([=]
			{
				videoCategory->Hide();
				audioCategory->Show();
				graphicsCategory->Hide();
			});
		graphicsCat->SetOnPressedEvent([=]
			{
				videoCategory->Hide();
				audioCategory->Hide();
				graphicsCategory->Show();
			});

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

		rtd::Picture* bg = gameOverCollection->AddElement<rtd::Picture>("MenuBG.png", draw_t(0, 0, width, height));
		rtd::Text* gameOverField = gameOverCollection->AddElement<rtd::Text>("Game Over", draw_text_t((width / 2.f) - (strlen("Game Over") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 5.f) - D2D1Core::GetDefaultFontSize(), strlen("Game Over") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));
		gameOverField->SetText("Game Over");

		rtd::Text* scoreField = gameOverCollection->AddElement<rtd::Text>("Score: 0", draw_text_t((width / 4.f) - (strlen("Score: ") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 2.f) - D2D1Core::GetDefaultFontSize(), strlen("Score: ") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));

		rtd::Text* waveField = gameOverCollection->AddElement<rtd::Text>("Waves: 0", draw_text_t((width / 1.5f) - (strlen("Waves: ") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 2.f) - D2D1Core::GetDefaultFontSize(), strlen("Waves: ") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));

		rtd::Button* mainMenuButton = gameOverCollection->AddElement<rtd::Button>("Button.png", draw_t((width / 2) - (widthScale / 8), height - (height / 6.f), widthScale / 4, height / 8));
		gameOverCollection->AddElement<rtd::Text>("Main Menu", draw_text_t((width / 2) - (width / 8), height - (height / 6.f), width / 4, height / 8));
		mainMenuButton->SetOnPressedEvent([=]
			{
				game->SetScene("JoinLobby");
			});

		scene.Add2DCollection(gameOverCollection, "GameOver");
	}

	void SetupLobbyJoinScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();

		float widthScale = height * (16.f / 9.f);
		const float paddingWidth = (widthScale / 64.f);
		const float paddingHeight = paddingWidth;

		Scene& scene = game->GetScene("JoinLobby");

		Collection2D* lobbySelectCollection = new Collection2D;
		Collection2D* nameCollection = new Collection2D;
		rtd::TextField* nameInputField = nameCollection->AddElement<rtd::TextField>(draw_text_t(width * 0.25f, height / 8, widthScale / 4, D2D1Core::GetDefaultFontSize()), 12);
		nameInputField->SetDescriptionText("Nickname");
		nameInputField->SetShape(Shapes::RECTANGLE_ROUNDED);

		rtd::Text* nameErrorText = nameCollection->AddElement<rtd::Text>("You must enter a valid name!", draw_text_t((width / 2.0f) - (width / 8), height * 0.8f, widthScale / 4.0f, height / 8.0f));
		nameErrorText->SetVisiblity(false);
#ifdef _DEBUG
		nameInputField->SetPresetText("Player");
#endif // DEBUG

		scene.Add2DCollection(nameCollection, "nameInput");

		Collection2D* lobbyCollection = new Collection2D;
		lobbyCollection->AddElement<rtd::Picture>("MenuBG.png", draw_t(0, 0, width, height));

		sm::Vector2 buttonSize = { widthScale / 4.f, height * 0.15f };
		float buttonPosY = height - (buttonSize.y + paddingHeight * 2.f);

		rtd::Button* exitButton = lobbyCollection->AddElement<rtd::Button>("No.png", draw_t(paddingWidth, paddingHeight, widthScale / 24, height / 14));
		exitButton->SetOnPressedEvent([=]
			{
				nameErrorText->SetVisiblity(false);
				game->m_client.Disconnect();
			});

		scene.Add2DCollection(lobbyCollection, "LobbyFields");

		float yPos = (height * 0.60f) / 5.0f;
		float yScale = ((height * 0.60f) / 5.0f) - paddingHeight;

		for (int i = 0; i < MAX_LOBBIES; i++)
		{
			rtd::LobbyUI* lobby = lobbySelectCollection->AddElement<rtd::LobbyUI>("Lobby " + std::to_string(i + 1), draw_t(width * 0.25f, height * 0.2f + yPos * i + 1.0f, widthScale * 0.5f, yScale));

			lobby->SetOnPressedEvent([=]()
				{
					if (nameInputField->RawGetBuffer()->length() > 0)
					{
						nameErrorText->SetVisiblity(false);
						game->m_playerName = *nameInputField->RawGetBuffer();

						// Update own name.
						dynamic_cast<rtd::Text*>(game->GetScene("Lobby").GetCollection("playerIcon1")->elements[1].get())->SetText(game->m_playerName);

						game->JoinLobby(i);
					}
					else
					{
						nameErrorText->SetVisiblity(true);
					}
					});
			}

			scene.Add2DCollection(lobbySelectCollection, "LobbySelect");
		}

		bool LoadMapColliders(Game * game)
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

		void LoadResources(Game * game)
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

		void LoadGameScene(Game * game)
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
