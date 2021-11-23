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

// Used to show and hide shopMenu
static bool toggle = false;

namespace sceneHelp
{
	Entity CreateLightEntity(Scene& scene, sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled)
	{
		Entity lightEntity = scene.CreateEntity();

		lightEntity.AddComponent<comp::Light>();
		lightEntity.GetComponent<comp::Light>()->lightData.position = pos;
		lightEntity.GetComponent<comp::Light>()->lightData.direction = dir;
		lightEntity.GetComponent<comp::Light>()->lightData.color = col;
		lightEntity.GetComponent<comp::Light>()->lightData.range = range;
		lightEntity.GetComponent<comp::Light>()->lightData.type = type;
		lightEntity.GetComponent<comp::Light>()->lightData.enabled = enabled;

		scene.GetLights()->EditLight(lightEntity.GetComponent<comp::Light>()->lightData, lightEntity.GetComponent<comp::Light>()->index);

		lightEntity.AddComponent<comp::BoundingSphere>();


		return lightEntity;
	}

	void CreateMainMenuScene(Game* game)
	{
		// Scene logic
		Scene& mainMenuScene = game->GetScene("MainMenu");
		SetupMainMenuScreen(game);

		Entity backgroundScene = mainMenuScene.CreateEntity();
		backgroundScene.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>("GameSceneAll.obj");
		backgroundScene.AddComponent<comp::Transform>();
		mainMenuScene.GetRegistry()->on_construct<comp::Light>().connect<&Lights::Add>(mainMenuScene.GetLights());

		CreateLightEntity(mainMenuScene, { 330.0f, 20.0f, -333.3f , 1.0f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 1000.0f, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(mainMenuScene, { 330.0f, 20.0f, -333.3f , 1.0f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 1000.0f, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(mainMenuScene, { 330.0f, 20.0f, -333.3f , 1.0f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 1000.0f, TypeLight::DIRECTIONAL, 1);

		comp::Transform test;
		test.position = { 330.0f, 1.0f, -333.3f };
		Entity blood = mainMenuScene.CreateEntity();
		blood.AddComponent<comp::Decal>(test);

		mainMenuScene.GetCurrentCamera()->Initialize(sm::Vector3(0, 0, 0), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)game->GetWindow()->GetWidth(), (float)game->GetWindow()->GetHeight()), CAMERATYPE::DEFAULT);
		mainMenuScene.GetCurrentCamera()->m_position = sm::Vector3(350.f, 30.f, -250.f);

		mainMenuScene.on<ESceneUpdate>([](const ESceneUpdate& e, Scene& scene)
			{
				static float d = 0.0f;
				d += e.dt;
				float angle = ((std::sin(d) + 1) * 0.5f) * dx::g_XMPi[0] * 0.05f;

				scene.GetCurrentCamera()->m_rotation = sm::Quaternion::CreateFromAxisAngle(sm::Vector3(0, 1, 0), angle + dx::XMConvertToRadians(170.0f - 30.f));

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

	void CreateGameScene(Game* game)
	{
		Scene& gameScene = game->GetScene("Game");
		SetupInGameScreen(game);

		//Construct collider meshes if colliders are added.
		gameScene.GetRegistry()->on_construct<comp::RenderableDebug>().connect<entt::invoke<&comp::RenderableDebug::InitRenderable>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingOrientedBox>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingSphere>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
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

		// DONT TOUCH
		/*
		CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 0, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 266.f, 29.f, -320.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 255.f, 142.f, 10.f, 0.f }, 20.f, TypeLight::POINT, 1);
		CreateLightEntity(gameScene, { 348.5f, 29.f, 325.5f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 255.f, 142.f, 10.f, 0.f }, 2.f, TypeLight::POINT, 1);
		*/

		//Entity dirLight = CreateLightEntity(gameScene, { 330.0f, 20.0f, -333.3f , 1.0f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 1000.0f, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 330.0f, 20.0f, -333.3f , 1.0f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 1000.0f, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 330.0f, 20.0f, -333.3f , 1.0f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 1000.0f, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 330.0f, 20.0f, -333.3f , 1.0f }, { -1.0f, -0.5f, 0.f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 1000.0f, TypeLight::DIRECTIONAL, 1);


		InputSystem::Get().SetCamera(gameScene.GetCurrentCamera());

		gameScene.on<ESceneUpdate>([cameraEntity, debugCameraEntity, game](const ESceneUpdate& e, Scene& scene)
			{
				IMGUI(
					ImGui::Begin("Scene");
				ImGui::Text("Game");
				ImGui::End();
				);

				if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
				{
					if (game->GetCurrentScene()->GetCollection("shopMenu")->GetState() == ElementState::OUTSIDE &&
						game->GetCurrentScene()->GetCollection("ScrolldownMenu")->GetState() == ElementState::OUTSIDE)
					{
						game->GetCurrentScene()->GetCollection("shopMenu")->Hide();
							toggle = false;
					}
				}

				// Prediction
				//engine->m_predictor.Predict(engine->GetScene("Game"));
				//GameSystems::RenderIsCollidingSystem(scene);
				GameSystems::UpdatePlayerVisuals(game);
				Systems::LightSystem(scene, e.dt);
#ifdef _DEBUG
				if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
				{
					if (scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::DEBUG)
					{
						scene.SetCurrentCameraEntity(cameraEntity);
						InputSystem::Get().SwitchMouseMode();
						LOG_INFO("Game Camera selected");
					}
					else if (scene.GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
					{
						scene.SetCurrentCameraEntity(debugCameraEntity);
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

		Collection2D* menuText = new Collection2D;
		menuText->AddElement<rtd::Text>("Welcome To Homehearth!", draw_text_t(0.f, 0.0f, width, height / 2.f));
		scene.Add2DCollection(menuText, "MenuText");

		Collection2D* connectFields = new Collection2D;
		rtd::TextField* ipField = connectFields->AddElement<rtd::TextField>(draw_text_t((width / 4), height * 0.55f, width * 0.25f, D2D1Core::GetDefaultFontSize()), 15, true);
		ipField->SetDescriptionText("IP Address:");
		rtd::TextField* portField = connectFields->AddElement<rtd::TextField>(draw_text_t(width / 4 + (width / 3.33f), height * 0.55f, width * 0.15f, D2D1Core::GetDefaultFontSize()), 6);
		portField->SetDescriptionText("Port:");
		rtd::Button* connectButton = connectFields->AddElement<rtd::Button>("Start.png", draw_t((width / 2) - (width / 8.f), height - (height * 0.25f), width / 4.f, height * 0.15f));
		scene.Add2DCollection(connectFields, "ConnectFields");
		//rtd::Button* exitButton = connectFields->AddElement<rtd::Button>("demoExitButton.png", draw_t(0.0f, 0.0f, width / 24, height / 16));
		//exitButton->SetOnPressedEvent([=] {
		//game->Shutdown();
		//	});

		Collection2D* scrolldownMenu = new Collection2D;
		rtd::Scroller* sc = scrolldownMenu->AddElement<rtd::Scroller>(draw_t(0.0f, -(height / 16) * 3.0f, width / 24.0f, (height / 16) * 3.0f), sm::Vector2(0, 0));
		sc->AddButton("No.png", draw_t(0.0f, -(height / 16), width / 24, height / 16))->SetOnPressedEvent([=] {
			game->Shutdown();
			});
		sc->AddButton("SettingsIcon.png", draw_t(0.0f, -(height / 16) * 2.0f, width / 24, height / 16))->SetOnPressedEvent([=] {
			game->SetScene("Options");
			});
		sc->SetPrimeButtonMeasurements(draw_t(0.0f, 0.0f, width / 24, height / 16));
		scene.Add2DCollection(scrolldownMenu, "ScrolldownMenu");

		rtd::Button* externalLinkBtn = connectFields->AddElement<rtd::Button>("Button.png", draw_t(width - width / 4.f, height - (height / 5), width / 8.f, height / 16));
		externalLinkBtn->GetText()->SetScale(0.5f);
		externalLinkBtn->GetText()->SetText("Give Feedback!");
		externalLinkBtn->SetOnPressedEvent([] {
			ShellExecuteA(NULL, "open", "https://docs.google.com/forms/d/e/1FAIpQLSfvyYTRNYaVHbg9Fa8H7xNXQGr2SWoaC9_GKZ7rSkuoNDjOMA/viewform?usp=sf_link", NULL, NULL, SW_SHOWNORMAL);
			});

#ifdef _DEBUG
		ipField->SetPresetText("localhost");
#endif
		portField->SetPresetText("4950");


		connectButton->SetOnPressedEvent([=]()
			{
				std::string* ip = ipField->RawGetBuffer();
				std::string* port = portField->RawGetBuffer();
				if (ip->length() > 0 && port->length() > 0)
				{
					if (game->m_client.Connect(ip->c_str(), std::stoi(port->c_str())))
					{
						rtd::TextField* nameInput = dynamic_cast<rtd::TextField*>(game->GetScene("JoinLobby").GetCollection("nameInput")->elements[0].get());
						nameInput->SetActive();
						game->SetScene("JoinLobby");
					}
				}
				else
				{
					LOG_WARNING("Please enter a valid ip/port");
				}
			});
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

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			Collection2D* playerHp = new Collection2D;

			// Initiate 3 healthbars. for each player.
			playerHp->AddElement<rtd::Healthbar>(draw_t(width / 8, (i * ((height / 12)) + (height / 32)), (width / 24), (height / 100)));

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

		Collection2D* timerCollection = new Collection2D;
		timerCollection->AddElement<rtd::Text>("0", draw_text_t(0, 0, width, height / 16.f));
		//rtd::Text* enemies = timerCollection->AddElement<rtd::Text>("Enemies", draw_text_t(width - (width / 8), 0, width / 8, height / 16.f));
		//enemies->SetVisiblity(false);
		scene.Add2DCollection(timerCollection, "timer");

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			Collection2D* nameCollection = new Collection2D;
			nameCollection->AddElement<rtd::Text>("Player", draw_text_t(0, 0, width / 14, height / 6));
			scene.Add2DCollection(nameCollection, "dynamicPlayer" + std::to_string(i + 1) + "namePlate");
			nameCollection->Hide();
		}

		Collection2D* money = new Collection2D;
		money->AddElement<rtd::MoneyUI>(draw_text_t(width - (width / 8.0f), D2D1Core::GetDefaultFontSize(), width / 8.0f, D2D1Core::GetDefaultFontSize()));
		scene.Add2DCollection(money, "MoneyUI");

		Collection2D* abilities = new Collection2D;
		rtd::AbilityUI* primary = abilities->AddElement<rtd::AbilityUI>(draw_t(width - width / 16.0f, height - height / 4.0f, width / 16.0f, height / 9.0f), D2D1::ColorF(0, 1.0f), "UI_sword.png");
		primary->SetActivateButton("LMB");
		rtd::AbilityUI* secondary = abilities->AddElement<rtd::AbilityUI>(draw_t(width - width / 8.0f, height - height / 9.0f, width / 16.0f, height / 9.0f), D2D1::ColorF(0, 1.0f), "UI_sword.png");
		secondary->SetActivateButton("RMB");
		rtd::AbilityUI* third = abilities->AddElement<rtd::AbilityUI>(draw_t((width / 2.f) - ((width / 16.0f) * 2.0f), height - height / 12.0f, width / 16.0f, height / 12.0f), D2D1::ColorF(0, 1.0f), "slashAbilityDemo.png");
		third->SetActivateButton("Q");
		rtd::AbilityUI* fourth = abilities->AddElement<rtd::AbilityUI>(draw_t((width / 2.f) - ((width / 16.0f)), height - height / 12.0f, width / 16.0f, height / 12.0f), D2D1::ColorF(0, 1.0f), "someRandomAbilityIdkDemo.png");
		fourth->SetActivateButton("E");
		rtd::AbilityUI* fifth = abilities->AddElement<rtd::AbilityUI>(draw_t((width / 2.f), height - height / 12.0f, width / 16.0f, height / 12.0f), D2D1::ColorF(0.0f, 1.0f), "healAbilityDemo.png");
		fifth->SetActivateButton("R");
		rtd::AbilityUI* sixth = abilities->AddElement<rtd::AbilityUI>(draw_t((width / 2.f) + ((width / 16.0f)), height - height / 12.0f, width / 16.0f, height / 12.0f), D2D1::ColorF(0, 1.0f), "slashAbilityDemo.png");
		sixth->SetActivateButton("C");
		scene.Add2DCollection(abilities, "AbilityUI");

		Collection2D* shopMenu = new Collection2D;
		Collection2D* scrolldownMenu = new Collection2D;
		rtd::Scroller* sc = scrolldownMenu->AddElement<rtd::Scroller>(draw_t(0.0f, -(height / 16) * 3.0f, width / 24.0f, (height / 16) * 3.0f), sm::Vector2(0, 0));
		sc->AddButton("No.png", draw_t(0.0f, -(height / 16), width / 24, height / 16))->SetOnPressedEvent([=] {
			game->Shutdown();
			});
		
		
		sc->AddButton("ShopIcon.png", draw_t(0.0f, -(height / 16) * 2.0f, width / 24, height / 16))->SetOnPressedEvent([=] {
			if (!toggle)
			{
				shopMenu->Show();
				toggle = true;
			}
			else
			{
				toggle = false;
				shopMenu->Hide();
			}
			});
		sc->SetPrimeButtonMeasurements(draw_t(0.0f, 0.0f, width / 24, height / 16));
		scene.Add2DCollection(scrolldownMenu, "ScrolldownMenu");

		shopMenu->AddElement<rtd::Canvas>(D2D1::ColorF(0.6f, 0.4f, 0.8f, 0.5f), draw_t(width / 24.0f, 0, width * 0.37f, height * 0.75f));
		//shopMenu->AddElement<rtd::Button>("demoExitButton.png", draw_t(width / 24, 0.0f, width / 24, height / 16))->SetOnPressedEvent([=] {
		//	
		//	shopMenu->Hide();

		//	});
		shopMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 24.f)  + ((width * 0.37f) * 0.5f) - width / 8.0f, height / 24.f, width / 4.0f, height / 8.0f))->SetOnPressedEvent([=] {

			game->UseShop(ShopItem::LONG_TOWER);

			});
		shopMenu->AddElement<rtd::Text>("Place Wide Towers", draw_text_t((width / 24.f) + ((width * 0.37f) * 0.5f) - width / 8.0f, height / 24.f, width / 4.0f, height / 8.0f));
		shopMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 24.f) + ((width * 0.37f) * 0.5f) - width / 8.0f, ((height / 24.f) * 2.0f) + height / 8.0f, width / 4.0f, height / 8.0f))->SetOnPressedEvent([=] {

			game->UseShop(ShopItem::SHORT_TOWER);

			});
		shopMenu->AddElement<rtd::Text>("Place Short Towers", draw_text_t((width / 24.f) + ((width * 0.37f) * 0.5f) - width / 8.0f, ((height / 24.f) * 2.0f) + height / 8.0f, width / 4.0f, height / 8.0f));
		shopMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 24.f) + ((width * 0.37f) * 0.5f) - width / 8.0f, ((height / 24.f) * 3.0f) + (height / 8.0f) * 2.0f, width / 4.0f, height / 8.0f))->SetOnPressedEvent([=] {

			game->UseShop(ShopItem::Tower_Upgrade);

			});
		shopMenu->AddElement<rtd::Text>("Upgrade Towers", draw_text_t((width / 24.f) + ((width * 0.37f) * 0.5f) - width / 8.0f, ((height / 24.f) * 3.0f) + (height / 8.0f) * 2.0f, width / 4.0f, height / 8.0f));
		shopMenu->Hide();
		scene.Add2DCollection(shopMenu, "shopMenu");
	}

	void SetupInLobbyScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();

		Scene& scene = game->GetScene("Lobby");

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			Collection2D* playerIcon = new Collection2D;

			playerIcon->AddElement<rtd::Canvas>(D2D1::ColorF(0.7f, 0.5f, 0.2f), draw_t(width / 16, (height / 12) * (i + 1) + (height / 12) * i, width / 4, height / 9));
			playerIcon->AddElement<rtd::Text>("Player " + std::to_string(i + 1), draw_text_t(width / 16, (height / 12) * (i + 1) + (height / 12) * i, width / 4, height / 9));
			playerIcon->AddElement<rtd::Picture>("WarriorIcon.png", draw_t((width / 8) + (width / 4), (height / 12) * (i + 1) + (height / 12) * i, width / 16, height / 9));
			scene.Add2DCollection(playerIcon, "playerIcon" + std::to_string(i + 1));

			// Hide everyother player other than first.
			if (i > 0)
				playerIcon->Hide();
		}

		Collection2D* classTextCanvas = new Collection2D;
		rtd::Picture* desc = classTextCanvas->AddElement<rtd::Picture>("WarriorDesc.png", draw_t((width / 2) + (width / 10.f), 10.0f, (width / 3.33f), height - (height / 6.0f)));
		scene.Add2DCollection(classTextCanvas, "ClassTextCanvas");

		Collection2D* lobbyDesc = new Collection2D;
		lobbyDesc->AddElement<rtd::Canvas>(D2D1::ColorF(1.0f, 1.0f, 1.0f), draw_t((width / 16), height - (height / 6), (width / 4), height / 12));

		// THIS ONE NEEDS A FUNCTION TO UPDATE LOBBY ID
		const std::string& lobbyString = "Lobby ID: XYZW";
		lobbyDesc->AddElement<rtd::Text>(lobbyString, draw_text_t((width / 16), height - (height / 6), (width / 4), height / 12));
		scene.Add2DCollection(lobbyDesc, "LobbyDesc");

		Collection2D* startGame = new Collection2D;
		rtd::Button* startGameButton = startGame->AddElement<rtd::Button>("NotReady.png", draw_t((width / 2) + (width / 10.f), height - (height / 5.0f), (width / 3.33f), (height / 6.f)), false);
		//rtd::Text* readyText = startGame->AddElement<rtd::Text>("Not ready", draw_text_t((width / 2) + (width / 10.f), height - (height / 5.0f), (width / 3.33f), (height / 6.f)));
		startGameButton->SetOnPressedEvent([=]()
			{
				comp::Player* player = game->GetLocalPlayer().GetComponent<comp::Player>();
				player->isReady = !player->isReady;
				game->SendStartGame();
			});
		scene.Add2DCollection(startGame, "StartGame");

		Collection2D* general = new Collection2D;
		general->AddElement<rtd::Canvas>(D2D1::ColorF(.2f, .2f, .2f), draw_t(0.0f, 0.0f, width, height));
		rtd::Button* exitButton = general->AddElement<rtd::Button>("No.png", draw_t(0.0f, 0.0f, width / 24, height / 16), false);
		exitButton->SetOnPressedEvent([=]()
			{
				comp::Player* player = game->GetLocalPlayer().GetComponent<comp::Player>();
				player->isReady = false;
				network::message<GameMsg> msg;
				msg.header.id = GameMsg::Lobby_Leave;
				msg << game->m_localPID << game->m_gameID;
				game->m_client.Send(msg);
				rtd::TextField* nameInput = dynamic_cast<rtd::TextField*>(game->GetScene("JoinLobby").GetCollection("nameInput")->elements[0].get());
				nameInput->SetActive();
			});
		scene.Add2DCollection(general, "AGeneral");

		Collection2D* classButtons = new Collection2D;
		rtd::Button* mageButton = classButtons->AddElement<rtd::Button>("WizardIcon.png", draw_t((width / 3.33f) + (float)(width / 20), height - (height / 6), width / 16, height / 9));
		// FIX WHAT CLASS SYMBOL PLAYER HAS LATER
		mageButton->SetOnPressedEvent([=]()
			{
				desc->SetTexture("WizardDesc.png");
				comp::Player* player = game->GetLocalPlayer().GetComponent<comp::Player>();
				player->classType = comp::Player::Class::MAGE;
				game->SendSelectedClass(player->classType);
			});
		rtd::Button* warriorButton = classButtons->AddElement<rtd::Button>("WarriorIcon.png", draw_t((width / 3.33f) + (width / 15.0f) + (float)(width / 16), height - (height / 6), width / 16, height / 9));
		warriorButton->SetOnPressedEvent([=]()
			{
				desc->SetTexture("WarriorDesc.png");
				comp::Player* player = game->GetLocalPlayer().GetComponent<comp::Player>();
				player->classType = comp::Player::Class::WARRIOR;
				game->SendSelectedClass(player->classType);
			});

		scene.Add2DCollection(classButtons, "ClassButtons");
	}

	void SetupOptionsScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();
		Scene& scene = game->GetScene("Options");

		Collection2D* helpText = new Collection2D;

		Collection2D* soundCollection = new Collection2D;
		rtd::Slider* sl = soundCollection->AddElement<rtd::Slider>(D2D1::ColorF(0.0f, 0.0f, 0.0f), draw_t((width / 2) - (width / 9), height / 5, width / 9, height / 16), &game->m_masterVolume);
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
		visualMenu->AddElement<rtd::Button>("Button.png", draw_t((width / 8.0f) * 5.0f, (height / 8.0f), width / 4.0f, height / 8.0f))->SetOnPressedEvent([=] {
			
			//visualMenu->Hide();
			//miscMenu->Show();
			
			});
		visualMenu->AddElement<rtd::Text>("Misc.", draw_t((width / 8.0f) * 5.0f, (height / 8.0f), width / 4.0f, height / 8.0f));


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
			resolutionMenu->Hide();

			});
		backButton->Hide();
		scene.Add2DCollection(backButton, "returnButton");

		helpText->AddElement<rtd::Text>("Insert super helpful text here for all the noobs.", draw_text_t(0.0f, 0.0f, width, height - (height / 8.0f)));
		//rtd::Button* goback = backButton->AddElement<rtd::Button>("Button.png", draw_t((width / 2.0f) - (width / 8.0f), height - (height / 4.0f), width / 4.0f, height / 8.0f));
		//backButton->AddElement<rtd::Text>("Go Back", draw_text_t((width / 2.0f) - (width / 8.0f), height - (height / 4.0f), width / 4.0f, height / 8.0f));
		//goback->SetOnPressedEvent([=] {

		//	helpText->Hide();
		//	menu->Show();
		//	backButton->Show();

		//	});
		helpText->Hide();
		scene.Add2DCollection(helpText, "HelpText");

	}

	void SetupLoadingScene(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();
		Scene& scene = game->GetScene("Loading");

		Collection2D* loadingScreen = new Collection2D;

		loadingScreen->AddElement<rtd::Picture>("oohstonefigures.jpg", (draw_t(0.0f, 0.0f, width, height)));
		loadingScreen->AddElement<rtd::Text>("Loading!", draw_text_t((width / 2.f) - (strlen("Loading!") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 2.f) - D2D1Core::GetDefaultFontSize(), strlen("Loading!") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));

		scene.Add2DCollection(loadingScreen, "LoadingScreen");
	}

	void SetupLobbyJoinScreen(Game* game)
	{
		const float width = (float)game->GetWindow()->GetWidth();
		const float height = (float)game->GetWindow()->GetHeight();
		Scene& scene = game->GetScene("JoinLobby");


		Collection2D* nameCollection = new Collection2D;
		rtd::TextField* nameInputField = nameCollection->AddElement<rtd::TextField>(draw_text_t((width / 2) - (width / 8), height / 8, width / 4, D2D1Core::GetDefaultFontSize()), 12, true);
		nameInputField->SetDescriptionText("Input Name");
		//nameInputField->SetPresetText("Noobie");
		scene.Add2DCollection(nameCollection, "nameInput");

		Collection2D* lobbyCollection = new Collection2D;

		rtd::TextField* lobbyField = lobbyCollection->AddElement<rtd::TextField>(draw_text_t(width / 8, height - (height / 3.33f), width / 4, D2D1Core::GetDefaultFontSize()));
		lobbyField->SetDescriptionText("Input Lobby ID");
		rtd::Button* startLobbyButton = lobbyCollection->AddElement<rtd::Button>("CreateLobby.png", draw_t(width / 2, height - (height / 6.f), width / 4, height / 8));
		//lobbyCollection->AddElement<rtd::Text>("Create Lobby", draw_text_t(width / 2, height - (height / 6.f), width / 4, height / 8));
		rtd::Button* lobbyButton = lobbyCollection->AddElement<rtd::Button>("joinLobby.png", draw_t(width / 8, height - (height / 6.f), width / 4, height / 8));
		//lobbyCollection->AddElement<rtd::Text>("Join Lobby", draw_text_t(width / 8, height - (height / 6.f), width / 4, height / 8));
		rtd::Button* exitButton = lobbyCollection->AddElement<rtd::Button>("No.png", draw_t(0.0f, 0.0f, width / 24, height / 16));

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
			else if (HouseRoof == filename)
			{
				modelID = ModelID::HOUSEROOF;
			}
			else if (Tree2 == filename)
			{
				modelID = ModelID::TREE2;
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

		return true;
	}

	void LoadAllAssets(Game* game)
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
			else if (HouseRoof == filename)
			{
				game->m_models[ModelID::HOUSEROOF].push_back(e);
			}
			else if (Tree2 == filename)
			{
				game->m_models[ModelID::TREE2].push_back(e);
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
	}
}