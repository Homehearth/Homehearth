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

namespace sceneHelp
{
	Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID)
	{
		Entity playerEntity = scene.CreateEntity();
		playerEntity.AddComponent<comp::Transform>()->position = { 0,0,1 };

		comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
		playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };

		comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
		playerEntity.AddComponent<comp::Network>()->id = playerID;

		renderable->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");
		return playerEntity;
	}

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

		return lightEntity;
	}

	void CreateMainMenuScene(Game* game)
	{
		// Scene logic
		Scene& mainMenuScene = game->GetScene("MainMenu");
		SetupMainMenuScreen(game);

		Entity backgroundScene = mainMenuScene.CreateEntity();
		backgroundScene.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>("GameScene.obj");
		backgroundScene.AddComponent<comp::Transform>();

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

	void CreateGameScene(Game* engine)
	{
		Scene& gameScene = engine->GetScene("Game");
		SetupInGameScreen(engine);

		//Construct collider meshes if colliders are added.
		gameScene.GetRegistry()->on_construct<comp::RenderableDebug>().connect<entt::invoke<&comp::RenderableDebug::InitRenderable>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingOrientedBox>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingSphere>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::Light>().connect<&Lights::Add>(gameScene.GetLights());

		// Setup Cameras
		Entity debugCameraEntity = gameScene.CreateEntity();
		debugCameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(200, 60, -320), sm::Vector3(200, 50, -350), sm::Vector3(0, 1, 0),
			sm::Vector2((float)engine->GetWindow()->GetWidth(), (float)engine->GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
		debugCameraEntity.AddComponent<comp::Tag<TagType::DEBUG_CAMERA>>();

		Entity cameraEntity = gameScene.CreateEntity();
		comp::Camera3D* gameCamera = cameraEntity.AddComponent<comp::Camera3D>();
		gameCamera->camera.Initialize(sm::Vector3(60, 100.f, 80), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)engine->GetWindow()->GetWidth(), (float)engine->GetWindow()->GetHeight()), CAMERATYPE::PLAY);
		gameCamera->camera.SetFOV(dx::XMConvertToRadians(30.f));
		cameraEntity.AddComponent<comp::Tag<TagType::CAMERA>>();

		gameScene.SetCurrentCameraEntity(cameraEntity);

		// DONT TOUCH
		//CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { 0.f, -1.f, -.5f, 0.f }, { 15.f, 15.f, 15.f, 0.f }, 0, TypeLight::DIRECTIONAL, 1);
		//CreateLightEntity(gameScene, { 310.f, 29.f, 305.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 255.f, 142.f, 10.f, 0.f }, 2.f, TypeLight::POINT, 1);
		//CreateLightEntity(gameScene, { 348.5f, 29.f, 325.5f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 255.f, 142.f, 10.f, 0.f }, 2.f, TypeLight::POINT, 1);

		InputSystem::Get().SetCamera(gameScene.GetCurrentCamera());

		gameScene.on<ESceneUpdate>([cameraEntity, debugCameraEntity, engine](const ESceneUpdate& e, Scene& scene)
			{

				IMGUI(
					ImGui::Begin("Scene");
				ImGui::Text("Game");
				ImGui::End();
				);

				// Prediction
				//engine->m_predictor.Predict(engine->GetScene("Game"));
				GameSystems::RenderIsCollidingSystem(scene);
				GameSystems::UpdatePlayerVisuals(scene);
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
						InputSystem::Get().SwitchMouseMode();
						LOG_INFO("Debug Camera selected");
					}
				}
#endif // DEBUG
			});
	}

}

Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID)
{
	Entity playerEntity = scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>();
	comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
	playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };
	comp::Velocity* playerVelocity = playerEntity.AddComponent<comp::Velocity>();
	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;
	playerEntity.AddComponent<comp::Network>()->id = playerID;

	renderable->model = ResourceManager::Get().GetResource<RModel>("cube.obj");

	return playerEntity;
}

void sceneHelp::SetupMainMenuScreen(Game* game)
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
	rtd::Button* connectButton = connectFields->AddElement<rtd::Button>("StartButton.png", draw_t((width / 2) - (width / 8.f), height - (height * 0.25f), width / 4.f, height * 0.15f));
	//rtd::Button* exitButton = connectFields->AddElement<rtd::Button>("demoExitButton.png", draw_t(0.0f, 0.0f, width / 24, height / 16));
	//exitButton->SetOnPressedEvent([=] {
	//game->Shutdown();
	//	});

	Collection2D* test = new Collection2D;
	rtd::Scroller* sc = test->AddElement<rtd::Scroller>(draw_t(0.0f, -480.0f, 160.0f, 480.0f), sm::Vector2(0, 0));
	sc->AddButton("demoExitButton.png", draw_t(32.0f, -32.0f, 64.0f, 32.0f))->SetOnPressedEvent([=] {
		game->Shutdown();
		});
	sc->AddButton("demo_options_button.png", draw_t(32.0f, -72.0f, 64.0f, 32.0f))->SetOnPressedEvent([=] {
		game->SetScene("Options");
		});
	scene.Add2DCollection(test, "test");

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

	scene.Add2DCollection(connectFields, "ConnectFields");

	connectButton->SetOnPressedEvent([=]()
		{
			std::string* ip = ipField->RawGetBuffer();
			std::string* port = portField->RawGetBuffer();
			if (ip->length() > 0 && port->length() > 0)
			{
				if (game->m_client.Connect(ip->c_str(), std::stoi(port->c_str())))
				{
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

void sceneHelp::SetupInGameScreen(Game* game)
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
		playerHp->AddElement<rtd::Healthbar>(draw_t(width / 8, (i * ((height / 12)) + (height / 32)), (width / 8), (height / 24)));

		// You and Friend text
		if (i == 0)
		{
			//playerHp->AddElement<rtd::Text>("You:", draw_text_t(0, (i * ((height / 12)) + (height / 32)), (width / 8), height / 16));
		}
		else
		{
			//playerHp->AddElement<rtd::Text>("Friend:", draw_text_t(0, (i * ((height / 12)) + (height / 32)), width / 8, height / 16));
			playerHp->Hide();
		}
		scene.Add2DCollection(playerHp, "player" + std::to_string(i + 1) + "Info");
	}

	Collection2D* timerCollection = new Collection2D;
	timerCollection->AddElement<rtd::Text>("0", draw_text_t(0, 0, width, height / 16.f));
	rtd::Text* enemies = timerCollection->AddElement<rtd::Text>("Enemies", draw_text_t(width - (width / 8), 0, width / 8, height / 16.f));
	enemies->SetVisiblity(false);
	scene.Add2DCollection(timerCollection, "timer");

	Collection2D* attackCollection = new Collection2D;
	attackCollection->AddElement<rtd::Text>("Attacks!", draw_text_t(0, height - (height / 6), (strlen("Attacks!") * D2D1Core::GetDefaultFontSize()) * 0.5f, D2D1Core::GetDefaultFontSize()));

	for (int i = 0; i < 1; i++)
	{
		attackCollection->AddElement<rtd::Picture>(texture2, draw_t(0, height - (height / 8), width / 12, height / 8));
	}
	scene.Add2DCollection(attackCollection, "attacks");

	Collection2D* buildCollection = new Collection2D;
	buildCollection->AddElement<rtd::Text>("Builds!", draw_text_t(width - (strlen("Builds!") * D2D1Core::GetDefaultFontSize()), height - (height / 6), strlen("Builds!") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));
	for (int i = 0; i < 1; i++)
	{
		buildCollection->AddElement<rtd::Picture>(texture2, draw_t((width - (width / 12)) - (i * (width / 12)), height - (height / 8), width / 16, height / 9));
	}
	scene.Add2DCollection(buildCollection, "builds");

	for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
	{
		Collection2D* nameCollection = new Collection2D;
		nameCollection->AddElement<rtd::Text>("Player", draw_text_t(0, 0, width / 14, height / 6));
		scene.Add2DCollection(nameCollection, "dynamicPlayer" + std::to_string(i + 1) + "namePlate");
		nameCollection->Hide();
	}

	Collection2D* buttons = new Collection2D;
	rtd::Button* exitButton = buttons->AddElement<rtd::Button>("demoExitButton.png", draw_t(0.0f, 0.0f, width / 24, height / 16));
	exitButton->SetOnPressedEvent([=] {
		game->m_client.Disconnect();
		game->Shutdown();
		});
	scene.Add2DCollection(buttons, "Buttons");
}

void sceneHelp::SetupInLobbyScreen(Game* game)
{
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();

	Scene& scene = game->GetScene("Lobby");

	for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
	{
		Collection2D* playerIcon = new Collection2D;

		playerIcon->AddElement<rtd::Canvas>(D2D1::ColorF(0.7f, 0.5f, 0.2f), draw_t(width / 16, (height / 12) * (i + 1) + (height / 12) * i, width / 4, height / 8));
		playerIcon->AddElement<rtd::Text>("Player " + std::to_string(i + 1), draw_text_t(width / 16, (height / 12) * (i + 1) + (height / 12) * i, width / 4, height / 8));
		playerIcon->AddElement<rtd::Picture>("warriorIconDemo.png", draw_t((width / 8) + (width / 4), (height / 12) * (i + 1) + (height / 12) * i, width / 16, height / 8));
		scene.Add2DCollection(playerIcon, "playerIcon" + std::to_string(i + 1));

		// Hide everyother player other than first.
		if (i > 0)
			playerIcon->Hide();
	}

	const std::string& desc = "##--Description--##";
	Collection2D* classTextCanvas = new Collection2D;
	classTextCanvas->AddElement<rtd::Canvas>(D2D1::ColorF(1.0f, 1.0f, 1.0f), draw_t(width / 2, 0, (width / 2.12f), height - (height / 4)));
	classTextCanvas->AddElement<rtd::Text>(desc, draw_text_t(width / 2, 0, width / 2, 24));
	scene.Add2DCollection(classTextCanvas, "ClassTextCanvas");

	Collection2D* warriorDesc = new Collection2D;
	const std::string& warriorString = "Warrior\nYour name is Carl, you have been training your whole life for this moment.\nIt started as a beautiful day in your village and all of a sudden monsters attacked!\nYou specialize in close combat and protecting your friends.";
	warriorDesc->AddElement<rtd::Text>(warriorString, draw_text_t(width / 2, 0, (width / 2.12f), height - (height / 4)));
	scene.Add2DCollection(warriorDesc, "WarriorText");

	const std::string& mageString = "Mage\nMr. Jackson how you've been?\nOur peaceful village is currently under attack and we would like to request your help!\nYour magic is quite frankly unchallenged being in long range surely is an advantage.\n\nYours truly,\n Dying villager.";
	Collection2D* mageDesc = new Collection2D;
	mageDesc->AddElement<rtd::Text>(mageString, draw_text_t(width / 2, 0, (width / 2.12f), height - (height / 4)));
	mageDesc->Hide();
	scene.Add2DCollection(mageDesc, "MageText");

	Collection2D* lobbyDesc = new Collection2D;
	lobbyDesc->AddElement<rtd::Canvas>(D2D1::ColorF(1.0f, 1.0f, 1.0f), draw_t((width / 16), height - (height / 6), (width / 4), height / 12));

	// THIS ONE NEEDS A FUNCTION TO UPDATE LOBBY ID
	const std::string& lobbyString = "Lobby ID: XYZW";
	lobbyDesc->AddElement<rtd::Text>(lobbyString, draw_text_t((width / 16), height - (height / 6), (width / 4), height / 12));
	scene.Add2DCollection(lobbyDesc, "LobbyDesc");

	Collection2D* startGame = new Collection2D;
	rtd::Button* startGameButton = startGame->AddElement<rtd::Button>("Button.png", draw_t((width / 2) + (width / 10.f), height - (height / 5.0f), (width / 3.33f), (height / 6.f)), false);
	rtd::Text* readyText = startGame->AddElement<rtd::Text>("Not ready", draw_text_t((width / 2) + (width / 10.f), height - (height / 5.0f), (width / 3.33f), (height / 6.f)));
	startGameButton->SetOnPressedEvent([=]()
		{
			comp::Player* player = game->GetLocalPlayer().GetComponent<comp::Player>();
			player->isReady = !player->isReady;
			game->SendStartGame();
		});
	scene.Add2DCollection(startGame, "StartGame");

	Collection2D* general = new Collection2D;
	general->AddElement<rtd::Canvas>(D2D1::ColorF(.2f, .2f, .2f), draw_t(0.0f, 0.0f, width, height));
	rtd::Button* exitButton = general->AddElement<rtd::Button>("demoExitButton.png", draw_t(0.0f, 0.0f, width / 24, height / 16), false);
	exitButton->SetOnPressedEvent([=]()
		{
			comp::Player* player = game->GetLocalPlayer().GetComponent<comp::Player>();
			player->isReady = false;
			network::message<GameMsg> msg;
			msg.header.id = GameMsg::Lobby_Leave;
			msg << game->m_localPID << game->m_gameID;
			game->m_client.Send(msg);
		});
	scene.Add2DCollection(general, "AGeneral");

	Collection2D* classButtons = new Collection2D;
	rtd::Button* mageButton = classButtons->AddElement<rtd::Button>("mageIconDemo.png", draw_t((width / 3.33f) + (float)(width / 20), height - (height / 6), width / 24, height / 16));
	// FIX WHAT CLASS SYMBOL PLAYER HAS LATER
	mageButton->SetOnPressedEvent([=]()
		{
			warriorDesc->Hide();
			mageDesc->Show();
		});
	rtd::Button* warriorButton = classButtons->AddElement<rtd::Button>("warriorIconDemo.png", draw_t((width / 3.33f) + (width / 20.0f) + (float)(width / 20), height - (height / 6), width / 24, height / 16));
	warriorButton->SetOnPressedEvent([=]()
		{
			mageDesc->Hide();
			warriorDesc->Show();
		});

	scene.Add2DCollection(classButtons, "ClassButtons");
}

void sceneHelp::SetupOptionsScreen(Game* game)
{
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();
	Scene& scene = game->GetScene("Options");


	Collection2D* soundCollection = new Collection2D;
	rtd::Slider* sl = soundCollection->AddElement<rtd::Slider>(D2D1::ColorF(0.0f, 0.0f, 0.0f), draw_t((width / 2) - (width / 9), height / 5, width / 9, height / 16), &game->m_masterVolume);
	sl->SetMinPos(sm::Vector2((width / 8) - (width / 9)));
	sl->SetMaxPos(sm::Vector2(width - (width / 8)));
	sl->SetExplanationText("Master Volume: ");
	scene.Add2DCollection(soundCollection, "Sounds");
}

void sceneHelp::SetupLoadingScene(Game* game)
{
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();
	Scene& scene = game->GetScene("Loading");

	Collection2D* loadingScreen = new Collection2D;

	loadingScreen->AddElement<rtd::Picture>("oohstonefigures.jpg", (draw_t(0.0f, 0.0f, width, height)));
	loadingScreen->AddElement<rtd::Text>("Loading!", draw_text_t((width / 2.f) - (strlen("Loading!") * D2D1Core::GetDefaultFontSize() * 0.5f), (height / 2.f) - D2D1Core::GetDefaultFontSize(), strlen("Loading!") * D2D1Core::GetDefaultFontSize(), D2D1Core::GetDefaultFontSize()));

	scene.Add2DCollection(loadingScreen, "LoadingScreen");
}

void sceneHelp::SetupLobbyJoinScreen(Game* game)
{
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();
	Scene& scene = game->GetScene("JoinLobby");


	Collection2D* nameCollection = new Collection2D;
	rtd::TextField* nameInputField = nameCollection->AddElement<rtd::TextField>(draw_text_t((width / 2) - (width / 8), height / 8, width / 4, D2D1Core::GetDefaultFontSize()), 12);
	nameInputField->SetDescriptionText("Input Name");
	//nameInputField->SetPresetText("Noobie");
	scene.Add2DCollection(nameCollection, "nameInput");

	Collection2D* lobbyCollection = new Collection2D;

	rtd::TextField* lobbyField = lobbyCollection->AddElement<rtd::TextField>(draw_text_t(width / 8, height - (height / 3.33f), width / 4, D2D1Core::GetDefaultFontSize()));
	lobbyField->SetDescriptionText("Input Lobby ID");
	rtd::Button* startLobbyButton = lobbyCollection->AddElement<rtd::Button>("Button.png", draw_t(width / 2, height - (height / 6.f), width / 4, height / 8));
	lobbyCollection->AddElement<rtd::Text>("Create Lobby", draw_text_t(width / 2, height - (height / 6.f), width / 4, height / 8));
	rtd::Button* lobbyButton = lobbyCollection->AddElement<rtd::Button>("Button.png", draw_t(width / 8, height - (height / 6.f), width / 4, height / 8));
	lobbyCollection->AddElement<rtd::Text>("Join Lobby", draw_text_t(width / 8, height - (height / 6.f), width / 4, height / 8));
	rtd::Button* exitButton = lobbyCollection->AddElement<rtd::Button>("demoExitButton.png", draw_t(0.0f, 0.0f, width / 24, height / 16));
	exitButton->SetOnPressedEvent([=] {
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
