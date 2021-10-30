#include "DemoScene.h"
#include "Canvas.h"
#include "Picture.h"
#include "Border.h"
#include "Button.h"
#include "Text.h"
#include "TextField.h"
#include "Slider.h"
#include "Collection2D.h"

namespace sceneHelp
{
	Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID)
	{
		Entity playerEntity = scene.CreateEntity();
		playerEntity.AddComponent<comp::Transform>();

		comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
		playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };

		comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
		playerEntity.AddComponent<comp::Network>()->id = playerID;

		renderable->model = ResourceManager::Get().GetResource<RModel>("cube.obj");
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
		backgroundScene.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>("Tree1.obj");
		backgroundScene.AddComponent<comp::Transform>();

		mainMenuScene.GetCurrentCamera()->Initialize(sm::Vector3(0, 60, -60), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)game->GetWindow()->GetWidth(), (float)game->GetWindow()->GetHeight()), CAMERATYPE::PLAY);
		mainMenuScene.GetCurrentCamera()->SetFollowEntity(backgroundScene);

		mainMenuScene.on<ESceneUpdate>([backgroundScene](const ESceneUpdate& e, Scene& scene)
			{
				backgroundScene.GetComponent<comp::Transform>()->rotation.y += e.dt;

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

	void CreateGameScene(Engine& engine)
	{
		Scene& gameScene = engine.GetScene("Game");
		SetupInGameScreen(gameScene);

		//Construct collider meshes if colliders are added.
		gameScene.GetRegistry()->on_construct<comp::RenderableDebug>().connect<entt::invoke<&comp::RenderableDebug::InitRenderable>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingOrientedBox>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::BoundingSphere>().connect<&entt::registry::emplace_or_replace<comp::RenderableDebug>>();
		gameScene.GetRegistry()->on_construct<comp::Light>().connect<&Lights::Add>(gameScene.GetLights());

		// Setup Cameras
		Entity debugCameraEntity = gameScene.CreateEntity();
		debugCameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
		debugCameraEntity.AddComponent<comp::Tag<TagType::DEBUG_CAMERA>>();

		Entity cameraEntity = gameScene.CreateEntity();
		cameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 20.f, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::PLAY);
		cameraEntity.AddComponent<comp::Tag<TagType::CAMERA>>();

		gameScene.SetCurrentCameraEntity(cameraEntity);

		CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { 1.f, -1.f, 0.f, 0.f }, { 10.f, 10.f, 10.f, 10.f }, 0, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 0.f, 8.f, -10.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 300.f, 300.f, 300.f, 300.f }, 75.f, TypeLight::POINT, 1);

		InputSystem::Get().SetCamera(gameScene.GetCurrentCamera());


		gameScene.on<ESceneUpdate>([cameraEntity, debugCameraEntity](const ESceneUpdate& e, Scene& scene)
			{

				IMGUI(
					ImGui::Begin("Scene");
				ImGui::Text("Game");
				ImGui::End();
				);

				GameSystems::RenderIsCollidingSystem(scene);

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
#if RENDER_IMGUI == 0
	// Setup main menu scene.
	Scene& scene = game->GetScene("MainMenu");
	float width = (float)game->GetWindow()->GetWidth();
	float height = (float)game->GetWindow()->GetHeight();

	Collection2D* menuText = new Collection2D;
	menuText->AddElement<rtd::Text>("Welcome To Homehearth!", draw_text_t(0.f, 0.0f, width, height / 2.f));
	scene.Add2DCollection(menuText, "MenuText");

	Collection2D* connectFields = new Collection2D;
	rtd::TextField* ipField = connectFields->AddElement<rtd::TextField>(draw_text_t(width / 3 - 50.f, 300.0f, 200.0f, 35.0f), 15, true);
	ipField->SetDescriptionText("IP Address:");
	rtd::TextField* portField = connectFields->AddElement<rtd::TextField>(draw_text_t(width / 3 + 200.f, 300.0f, 100.0f, 35.0f), 6);
	portField->SetDescriptionText("Port:");
	rtd::Button* connectButton = connectFields->AddElement<rtd::Button>("StartButton.png", draw_t((width / 2) - (width / 8.f), height - (height * 0.25f), width / 4.f, height * 0.15f));

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

	//// Adds a button and names it exit game button.
	//rtd::Button* exitGameButton = new rtd::Button("demo_exit_button.png", draw_t((Width / 8), (height / 4) + 200, 350.0f, 150.0f));
	//// Adds a border around the button and sets the color to black.
	//exitGameButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	//scene.Insert2DElement(exitGameButton);
	//exitGameButton->SetOnPressedEvent([=] { game->Shutdown(); });
#endif
}

void sceneHelp::SetupInGameScreen(Scene& scene)
{
#if RENDER_IMGUI == 0
	//// Temp textures
	//const std::string& texture1 = "Leafs3.png";
	//const std::string& texture2 = "LampFlowerLeafs.png";
	//const std::string& texture3 = "LeafsDark.png";
	//const std::string& texture4 = "Light.png";

	//// Player 1
	//// Temp 'heart' displayed as leaf
	//rtd::Picture* heart1 = new rtd::Picture(texture1, draw_t(90.0f, 10.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(heart1);

	//rtd::Picture* heart2 = new rtd::Picture(texture1, draw_t(174.0f, 10.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(heart2);

	//rtd::Picture* heart3 = new rtd::Picture(texture1, draw_t(258.0f, 10.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(heart3);

	//rtd::Text* youText = new rtd::Text("You:", draw_text_t(5.0f, 30.0f, 60.0f, 20.0f));
	//scene.Insert2DElement(youText);

	//// Player 2
	//rtd::Picture* heart4 = new rtd::Picture(texture1, draw_t(90.0f, 72.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(heart4);

	//rtd::Picture* heart5 = new rtd::Picture(texture1, draw_t(174.0f, 72.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(heart5);

	//rtd::Picture* heart6 = new rtd::Picture(texture1, draw_t(258.0f, 72.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(heart6);

	//rtd::Text* friendText = new rtd::Text("Friend:", draw_text_t(5.0f, 95.0f, 84.0f, 20.0f));
	//scene.Insert2DElement(friendText);

	////Timer text
	//const std::string& timerText = "1:20";
	//rtd::Text* timer = new rtd::Text(timerText, draw_text_t(436.0f, 24.0f, 96.0f, 24.0f));
	//scene.Insert2DElement(timer, "TimerText");

	//// Attacks
	//rtd::Text* attacksText = new rtd::Text("Attacks!", draw_text_t(24.0f, 412.0f, 96.0f, 24.0f));
	//scene.Insert2DElement(attacksText);

	//rtd::Picture* attack1 = new rtd::Picture(texture2, draw_t(24.f, 448.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(attack1);

	//rtd::Picture* attack2 = new rtd::Picture(texture3, draw_t(98.f, 448.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(attack2);

	//rtd::Picture* attack3 = new rtd::Picture(texture4, draw_t(172.f, 448.0f, 64.0f, 64.0f));
	//attack3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	//attack2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	//attack1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	//scene.Insert2DElement(attack3);

	//// Builds
	//rtd::Text* buildText = new rtd::Text("Builds!", draw_text_t(700.0f, 412.0f, 96.0f, 24.0f));
	//scene.Insert2DElement(buildText);

	//rtd::Picture* build1 = new rtd::Picture(texture2, draw_t(700.f, 448.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(build1);

	//rtd::Picture* build2 = new rtd::Picture(texture3, draw_t(774.f, 448.0f, 64.0f, 64.0f));
	//scene.Insert2DElement(build2);
	//rtd::Picture* build3 = new rtd::Picture(texture4, draw_t(848.f, 448.0f, 64.0f, 64.0f));
	//build3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	//build2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	//build1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	//scene.Insert2DElement(build3);
#endif
}

void sceneHelp::SetupInLobbyScreen(Game* game)
{
#if RENDER_IMGUI == 0
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();

	Scene& scene = game->GetScene("Lobby");

	for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
	{
		Collection2D* playerIcon = new Collection2D;

		playerIcon->AddElement<rtd::Canvas>(D2D1::ColorF(0.7f, 0.5f, 0.2f), draw_t(25.0f, 100.0f, 300.0f, 64.0f));
		playerIcon->AddElement<rtd::Text>("Player " + std::to_string(i + 1), draw_text_t(0.0f, 100.0f, 8 * 24.0f, 64.0f));
		playerIcon->AddElement<rtd::Picture>("warriorIconDemo.png", draw_t(350.0f, 125.0f, 64.0f, 64.0f));
		scene.Add2DCollection(playerIcon, "playerIcon" + std::to_string(i + 1));
	}

	Collection2D* general = new Collection2D;
	general->AddElement<rtd::Canvas>(D2D1::ColorF(.2f, .2f, .2f), draw_t(0.0f, 0.0f, width, height));
	rtd::Button* exitButton = general->AddElement<rtd::Button>("demoExitButton.png", draw_t(0.0f, 0.0f, 32.0f, 32.0f), false);
	exitButton->SetOnPressedEvent([=]()
		{
			network::message<GameMsg> msg;
			msg.header.id = GameMsg::Lobby_Leave;
			msg << game->m_localPID << game->m_gameID;
			game->m_client.Send(msg);
		});
	scene.Add2DCollection(general, "AGeneral");

	const std::string& desc = "##--Description--##";
	Collection2D* classTextCanvas = new Collection2D;
	classTextCanvas->AddElement<rtd::Canvas>(D2D1::ColorF(1.0f, 1.0f, 1.0f), draw_t(width / 2, 0, (width / 2.12f), height - (height / 4)));
	classTextCanvas->AddElement<rtd::Text>(desc, draw_text_t(width / 2, 0, width / 2, 24));
	scene.Add2DCollection(classTextCanvas, "ClassTextCanvas");

	Collection2D* warriorDesc = new Collection2D;
	const std::string& warriorString = "Warrior\nThe Warrior specializes in CQ Combat.";
	warriorDesc->AddElement<rtd::Text>(warriorString, draw_text_t(width / 2, 0, (width / 2.12f), height - (height / 4)));
	scene.Add2DCollection(warriorDesc, "WarriorText");

	const std::string& mageString = "Mage\nThis weak character is good for nothing please choose the warrior instead.";
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
	rtd::Button* startGameButton = startGame->AddElement<rtd::Button>("StartButton.png", draw_t((width / 2) + (width / 10.f), height - (height / 5.0f), (width / 3.33f), (height / 6.f)), false);
	startGameButton->SetOnPressedEvent([=]()
		{
			game->SendStartGame();
		});
	scene.Add2DCollection(startGame, "StartGame");

	Collection2D* classButtons = new Collection2D;
	rtd::Button* mageButton = classButtons->AddElement<rtd::Button>("mageIconDemo.png", draw_t(350.0f, 454.0f, 32.0f, 32.0f));
	// FIX WHAT CLASS SYMBOL PLAYER HAS LATER
	mageButton->SetOnPressedEvent([=]()
		{
			warriorDesc->Hide();
			mageDesc->Show();
		});
	rtd::Button* warriorButton = classButtons->AddElement<rtd::Button>("warriorIconDemo.png", draw_t(392.0f, 454.0f, 32.0f, 32.0f));
	warriorButton->SetOnPressedEvent([=]()
		{
			mageDesc->Hide();
			warriorDesc->Show();
		});

	scene.Add2DCollection(classButtons, "ClassButtons");
#endif
}

void sceneHelp::SetupOptionsScreen(Scene& scene)
{

}

void sceneHelp::SetupLoadingScene(Game* game)
{
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();
	Scene& scene = game->GetScene("Loading");

	Collection2D* loadingScreen = new Collection2D;

	loadingScreen->AddElement<rtd::Picture>("oohstonefigures.jpg", (draw_t(0.0f, 0.0f,
		width, height)));
	loadingScreen->AddElement<rtd::Text>("Loading!", draw_text_t((width / 2.f), (height / 2.f) - 24.f, strlen("Loading!") * 24.f, 24.f));

	scene.Add2DCollection(loadingScreen, "LoadingScreen");
}

void sceneHelp::SetupLobbyJoinScreen(Game* game)
{
#if RENDER_IMGUI == 0
	const float width = (float)game->GetWindow()->GetWidth();
	const float height = (float)game->GetWindow()->GetHeight();
	Scene& scene = game->GetScene("JoinLobby");

	Collection2D* lobbyCollection = new Collection2D;

	rtd::TextField* lobbyField = lobbyCollection->AddElement<rtd::TextField>(draw_text_t(width / 8, height - (height / 3), 200.0f, 35.0f));
	lobbyField->SetDescriptionText("Input Lobby ID");
	rtd::Button* lobbyButton = lobbyCollection->AddElement<rtd::Button>("StartButton.png", draw_t(width / 2, height - (height / 4), 300.0f, 125.0f));

	lobbyButton->SetOnPressedEvent([=]()
		{
			std::string* lobbyString = lobbyField->RawGetBuffer();

			if (lobbyString)
			{
				if (lobbyString->size() == 0)
				{
					game->CreateLobby();
					game->SetScene("Loading");
				}
				else
				{
					game->JoinLobby(std::stoi(*lobbyString));
					game->SetScene("Loading");
				}
			}
		});

	scene.Add2DCollection(lobbyCollection, "LobbyFields");

#endif
}
