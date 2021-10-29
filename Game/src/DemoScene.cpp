#include "DemoScene.h"
#include "Canvas.h"
#include "Picture.h"
#include "Border.h"
#include "Button.h"
#include "Text.h"
#include "TextField.h"
#include "Slider.h"

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

	void CreateMainMenuScene(Engine& engine)
	{

		// Scene logic
		Scene& mainMenuScene = engine.GetScene("MainMenu");
		SetupMainMenuScreen(engine, mainMenuScene);
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

	void CreateConnectScene(Game* game)
	{
		SetupConnectScreen(game);
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
		cameraEntity.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 2.8f, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0),
			sm::Vector2((float)engine.GetWindow()->GetWidth(), (float)engine.GetWindow()->GetHeight()), CAMERATYPE::PLAY);
		cameraEntity.AddComponent<comp::Tag<TagType::CAMERA>>();

		gameScene.SetCurrentCameraEntity(cameraEntity);

		CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { 1.f, -1.f, 0.f, 0.f }, { 10.f, 10.f, 10.f, 10.f }, 0, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 0.f, 8.f, -10.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 300.f, 300.f, 300.f, 300.f }, 75.f, TypeLight::POINT, 1);

		InputSystem::Get().SetCamera(gameScene.GetCurrentCamera());


		gameScene.on<ESceneUpdate>([cameraEntity, debugCameraEntity](const ESceneUpdate& e, Scene& scene)
			{
				scene.GetCurrentCamera()->Update(e.dt);

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

void sceneHelp::SetupMainMenuScreen(Engine& engine, Scene& scene)
{
#if RENDER_IMGUI == 0
	// Setup main menu scene.

	// Adds text to the menu screen.
	rtd::Text* welcomeText = new rtd::Text("Welcome To Homehearth!", draw_text_t(575.0f, 50.0f, 300.0f, 100.0f));
	scene.Insert2DElement(welcomeText);
	std::string welcomeString = "In this game you will face against very dangerous foes while defending the righteous village from its dark fate! Take up arms and fight your way to victory champion! Join our discord and twitter to get official news about the new upcoming technological wonder game! Sign up for RTX exclusive version at our website!";

	// Adds text to the menu screen.
	rtd::Text* gameInfoText = new rtd::Text(welcomeString, draw_text_t(550.0f, 0.0f, 350.0f, 550.0f));
	scene.Insert2DElement(gameInfoText);
	// Adds a button and names it start game button.
	rtd::Button* startGameButton = new rtd::Button("StartButton.png", draw_t(100.0f, 100.0f, 350.0f, 150.0f));
	scene.Insert2DElement(startGameButton);

	startGameButton->SetOnPressedEvent([&] { engine.SetScene("ConnectMenu"); });

	// Adds a button and names it exit game button.
	rtd::Button* exitGameButton = new rtd::Button("demo_exit_button.png", draw_t(100.0f, 325.0f, 350.0f, 150.0f));
	// Adds a border around the button and sets the color to black.
	exitGameButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	scene.Insert2DElement(exitGameButton);
	exitGameButton->SetOnPressedEvent([&] { engine.Shutdown(); });
#endif
}

void sceneHelp::SetupInGameScreen(Scene& scene)
{
#if RENDER_IMGUI == 0
	// Temp textures
	const std::string& texture1 = "Leafs3.png";
	const std::string& texture2 = "LampFlowerLeafs.png";
	const std::string& texture3 = "LeafsDark.png";
	const std::string& texture4 = "Light.png";

	// Player 1
	// Temp 'heart' displayed as leaf
	rtd::Picture* heart1 = new rtd::Picture(texture1, draw_t(90.0f, 10.0f, 64.0f, 64.0f));
	scene.Insert2DElement(heart1);

	rtd::Picture* heart2 = new rtd::Picture(texture1, draw_t(174.0f, 10.0f, 64.0f, 64.0f));
	scene.Insert2DElement(heart2);

	rtd::Picture* heart3 = new rtd::Picture(texture1, draw_t(258.0f, 10.0f, 64.0f, 64.0f));
	scene.Insert2DElement(heart3);

	rtd::Text* youText = new rtd::Text("You:", draw_text_t(5.0f, 30.0f, 60.0f, 20.0f));
	scene.Insert2DElement(youText);

	// Player 2
	rtd::Picture* heart4 = new rtd::Picture(texture1, draw_t(90.0f, 72.0f, 64.0f, 64.0f));
	scene.Insert2DElement(heart4);

	rtd::Picture* heart5 = new rtd::Picture(texture1, draw_t(174.0f, 72.0f, 64.0f, 64.0f));
	scene.Insert2DElement(heart5);

	rtd::Picture* heart6 = new rtd::Picture(texture1, draw_t(258.0f, 72.0f, 64.0f, 64.0f));
	scene.Insert2DElement(heart6);

	rtd::Text* friendText = new rtd::Text("Friend:", draw_text_t(5.0f, 95.0f, 84.0f, 20.0f));
	scene.Insert2DElement(friendText);

	//Timer text
	const std::string& timerText = "1:20";
	rtd::Text* timer = new rtd::Text(timerText, draw_text_t(436.0f, 24.0f, 96.0f, 24.0f));
	scene.Insert2DElement(timer, "TimerText");

	// Attacks
	rtd::Text* attacksText = new rtd::Text("Attacks!", draw_text_t(24.0f, 412.0f, 96.0f, 24.0f));
	scene.Insert2DElement(attacksText);

	rtd::Picture* attack1 = new rtd::Picture(texture2, draw_t(24.f, 448.0f, 64.0f, 64.0f));
	scene.Insert2DElement(attack1);

	rtd::Picture* attack2 = new rtd::Picture(texture3, draw_t(98.f, 448.0f, 64.0f, 64.0f));
	scene.Insert2DElement(attack2);

	rtd::Picture* attack3 = new rtd::Picture(texture4, draw_t(172.f, 448.0f, 64.0f, 64.0f));
	attack3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	attack2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	attack1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	scene.Insert2DElement(attack3);

	// Builds
	rtd::Text* buildText = new rtd::Text("Builds!", draw_text_t(700.0f, 412.0f, 96.0f, 24.0f));
	scene.Insert2DElement(buildText);

	rtd::Picture* build1 = new rtd::Picture(texture2, draw_t(700.f, 448.0f, 64.0f, 64.0f));
	scene.Insert2DElement(build1);

	rtd::Picture* build2 = new rtd::Picture(texture3, draw_t(774.f, 448.0f, 64.0f, 64.0f));
	scene.Insert2DElement(build2);
	rtd::Picture* build3 = new rtd::Picture(texture4, draw_t(848.f, 448.0f, 64.0f, 64.0f));
	build3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	build2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	build1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	scene.Insert2DElement(build3);
#endif
}

void sceneHelp::SetupInLobbyScreen(Game* game)
{
#if RENDER_IMGUI == 0
	Scene& scene = game->GetScene("Lobby");
	const std::string& warriorString = "Warrior\nThe Warrior specializes in CQ Combat.";
	const std::string& mageString = "Mage\nThis weak character is good for nothing please choose the warrior instead.";

	rtd::Canvas* backgroundCanvas = new rtd::Canvas(D2D1::ColorF(.2f, .2f, .2f), draw_t(0.0f, 0.0f, 3000.0f, 3000.0f));
	scene.Insert2DElement(backgroundCanvas, "Canvas1");

	rtd::Canvas* textCanvas = new rtd::Canvas(D2D1::ColorF(1.0f, 1.0f, 1.0f), draw_t(580.0f, 10.0f, 350.0f, 400.0f));
	scene.Insert2DElement(textCanvas, "Canvas2");

	rtd::Canvas* lobbyIdCanvas = new rtd::Canvas(D2D1::ColorF(1.0f, 1.0f, 1.0f), draw_t(30.0f, 420.0f, 275.0f, 100.0f));
	scene.Insert2DElement(lobbyIdCanvas, "Canvas3");

	rtd::Button* startGameButton = new rtd::Button("StartButton.png", draw_t(625.0f, 420.0f, 250.0f, 100.0f), false);
	scene.Insert2DElement(startGameButton, "readyButton");
	startGameButton->SetOnPressedEvent([=] {
		
		game->SendStartGame();

		});

	const std::string& dt = "##--Description--##";
	rtd::Text* descText = new rtd::Text(dt, draw_text_t(530.0f, 20.0f, dt.length() * 24.0f, 24));
	scene.Insert2DElement(descText, "DescriptionText");


	rtd::Text* warriorText = new rtd::Text(warriorString, draw_text_t(580.0f, 30.0f, 350.0f, 370.0f));
	scene.Insert2DElement(warriorText, "warriorText");

	// Init mage text but set it to no render.
	rtd::Text* mageText = new rtd::Text(mageString, draw_text_t(580.0f, 30.0f, 350.0f, 370.0f));
	scene.Insert2DElement(mageText, "mageText");
	mageText->SetVisiblity(false);

	const std::string& lobbyString = "Lobby ID: XYZW";
	rtd::Text* lobbyIdText = new rtd::Text(lobbyString, draw_text_t(0.0f, 460.0f, lobbyString.length() * 24.0f, 24.0f));
	scene.Insert2DElement(lobbyIdText, "lobbyID");
	rtd::Button* mageButton = new rtd::Button("mageIconDemo.png", draw_t(350.0f, 454.0f, 32.0f, 32.0f));
	scene.Insert2DElement(mageButton, "mageButton");

	rtd::Button* warriorButton = new rtd::Button("warriorIconDemo.png", draw_t(392.0f, 454.0f, 32.0f, 32.0f));
	scene.Insert2DElement(warriorButton, "warriorButton");
	// Player slots
	for (int i = 0; i < 2; i++)
	{
		const std::string& playerString = "Player " + std::to_string(i + 1);
		rtd::Text* playerText = new rtd::Text(playerString, draw_text_t(0.0f, (i + 1) * 100.0f + ((i + 1) * 25.0f), playerString.length() * 24.0f, 64.0f));
		rtd::Canvas* playerCanvas = new rtd::Canvas(D2D1::ColorF(0.7f, 0.5f, 0.2f), draw_t(25.0f, (i + 1) * 100.0f + ((i + 1) * 25.0f), 300.0f, 64.0f));
		scene.Insert2DElement(playerCanvas, "Canvas" + std::to_string(i + 4));
		scene.Insert2DElement(playerText, "PlayerText" + std::to_string(i + 1));

		// Hide the other player at the start of game.
		if (i == 1)
		{
			playerText->SetVisiblity(false);
			playerCanvas->SetVisiblity(false);
		}
	}

	rtd::Picture* player1Symbol = new rtd::Picture("warriorIconDemo.png", draw_t(350.0f, 125.0f, 64.0f, 64.0f));
	scene.Insert2DElement(player1Symbol, "player1Symbol");

	rtd::Picture* player2Symbol = new rtd::Picture("warriorIconDemo.png", draw_t(350.0f, 250.0f, 64.0f, 64.0f));
	scene.Insert2DElement(player2Symbol, "player2Symbol");
	player2Symbol->SetVisiblity(false);

	rtd::Text* homehearthText = new rtd::Text("Homehearth", draw_text_t(25.0f, 25.0f, 200.0f, 50.0f));
	scene.Insert2DElement(homehearthText, "TextHomeHearth");
	rtd::Button* exitToMainMenu = new rtd::Button("demoExitButton.png", draw_t(0.0f, 0.0f, 32.0f, 32.0f), false);
	scene.Insert2DElement(exitToMainMenu, "exitButton");

	exitToMainMenu->SetOnPressedEvent([=] {
		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Leave;
		msg << game->m_localPID << game->m_gameID;
		game->m_client.Send(msg);
		});

#endif
}

void sceneHelp::SetupOptionsScreen(Scene& scene)
{
}

void sceneHelp::SetupConnectScreen(Game* game)
{
	Scene& connectScene = game->GetScene("ConnectMenu");

	const unsigned int width = game->GetWindow()->GetWidth(), height = game->GetWindow()->GetHeight();

	rtd::TextField* ipField = new rtd::TextField(draw_text_t(width / 3 - 50.f, 100.0f, 200.0f, 35.0f), 15, true);
	connectScene.Insert2DElement(ipField, "ipField");
	ipField->SetDescriptionText("IP address:");

	rtd::TextField* portField = new rtd::TextField(draw_text_t(width / 3 + 200.f, 100.0f, 100.0f, 35.0f), 6);
	connectScene.Insert2DElement(portField, "portField");
	portField->SetDescriptionText("Port:");
	const char* t = ipField->RawGetBuffer()->c_str();
	const char* p = portField->RawGetBuffer()->c_str();

	rtd::Button* connectButton = new rtd::Button("StartButton.png", draw_t((float)width / 2 - 150.f, (float)height - (float)height / 3, 300.0f, 100.0f));
	connectScene.Insert2DElement(connectButton, "connectButton");
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

void sceneHelp::SetupLoadingScene(Game* game)
{
	Scene& scene = game->GetScene("Loading");
	rtd::Picture* background = new rtd::Picture("oohstonefigures.jpg", draw_t(0.0f, 0.0f, game->GetWindow()->GetWidth(), game->GetWindow()->GetHeight()));
	scene.Insert2DElement(background, "Background");

	
	rtd::Text* loadingText = new rtd::Text("LOADING...", draw_text_t(game->GetWindow()->GetWidth() / 2.0f, game->GetWindow()->GetHeight() / 2.0f, 100.0f, 25.0f));
	scene.Insert2DElement(loadingText, "LoadingText");

}

void sceneHelp::SetupLobbyJoinScreen(Game* game)
{
#if RENDER_IMGUI == 0

	Scene& joinScene = game->GetScene("JoinLobby");

	rtd::TextField* lobbyField = new rtd::TextField(draw_text_t(100.0f, 300.0f, 200.0f, 35.0f));
	//rtd::TextField * lobbyField = new rtd::TextField(draw_text_t((float)(rand() % 1000) / 2, (float)(rand() % 1000) / 4, 200.0f, 35.0f));
	lobbyField->SetDescriptionText("Input Lobby ID");
	joinScene.Insert2DElement(lobbyField, "lobbyField");
	rtd::Button* hostLobbyButton = new rtd::Button("StartButton.png", draw_t(500.0f, 300.0f, 300.0f, 125.0f));
	//rtd::Button* hostLobbyButton = new rtd::Button("StartButton.png", draw_t((float)(rand() % 1000) / 2, (float)(rand() % 1000) / 4, 300.0f, 125.0f));
	joinScene.Insert2DElement(hostLobbyButton, "startLobby");

	// Start or Join Lobby
	joinScene.GetElement<rtd::Button>("startLobby")->SetOnPressedEvent([=]
		{
			Scene& joinScene = game->GetScene("JoinLobby");
			std::string* lobbyString = joinScene.GetElement<rtd::TextField>("lobbyField")->RawGetBuffer();

			if (lobbyString)
			{
				if ((int)lobbyString->size() <= 0)
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

#endif
}
