#include "DemoScene.h"

void SetupServerConnectScreen(Window* pWindow);
void SetupMainMenuScreen(Window* pWindow);
void SetupInLobbyScreen();
void SetupInGameScreen();
void SetupOptionsScreen();

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
		SetupMainMenuScreen(engine.GetWindow());
		rtd::Handler2D::Get().SetVisibilityAll(false);
		SetupServerConnectScreen(engine.GetWindow());
		//SetupLobbyJoinScreen();
		// Scene logic
		Scene& mainMenuScene = engine.GetScene("MainMenu");
		mainMenuScene.on<ESceneUpdate>([](const ESceneUpdate& e, Scene& scene)
			{
				IMGUI(
					ImGui::Begin("Scene");
				ImGui::Text("MainMenu");
				ImGui::End();
				);
			});
	}

	void CreateLobbyScene(Engine& engine)
	{

	}

	void CreateGameScene(Engine& engine)
	{


		Scene& gameScene = engine.GetScene("Game");
		SetupInGameScreen();

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

		for (int i = 0; i < 5; i++)
		{
			// Debug Chest
			Entity chest = gameScene.CreateEntity();
			comp::Transform* transform = chest.AddComponent<comp::Transform>();
			transform->position.z = 5.0f * static_cast<float>(i) + 0.2f;
			comp::Velocity* chestVelocity = chest.AddComponent<comp::Velocity>();
			comp::BoundingOrientedBox* sphere = chest.AddComponent<comp::BoundingOrientedBox>();
			sphere->Center = transform->position;
			sphere->Extents = sm::Vector3(2.0f);
			comp::Renderable* renderable2 = chest.AddComponent<comp::Renderable>();

			renderable2->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");
		}

		CreateLightEntity(gameScene, { 0.f, 0.f, 0.f, 0.f }, { 1.f, -1.f, 0.f, 0.f }, { 10.f, 10.f, 10.f, 10.f }, 0, TypeLight::DIRECTIONAL, 1);
		CreateLightEntity(gameScene, { 0.f, 8.f, -10.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 300.f, 300.f, 300.f, 300.f }, 75.f, TypeLight::POINT, 1);

		Entity demoScene = gameScene.CreateEntity();
		comp::Transform* DemoSceneTransform = demoScene.AddComponent<comp::Transform>();
		comp::Renderable* demoSceneModel = demoScene.AddComponent<comp::Renderable>();
		demoSceneModel->model = ResourceManager::Get().GetResource<RModel>("Demoscene.fbx");


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


void SetupServerConnectScreen(Window* pWindow)
{
	const unsigned int width = pWindow->GetWidth(), height = pWindow->GetHeight();
#if RENDER_IMGUI == 0


	rtd::TextField* ipField = new rtd::TextField(draw_text_t(width / 3 - 50.f, 100.0f, 200.0f, 35.0f));
	ipField->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	rtd::Handler2D::Get().InsertElement(ipField, "serverConnect");
	ipField->SetName("ipBuffer");
	ipField->GetText()->SetText("Input IP adress");
	
	rtd::TextField* portField = new rtd::TextField(draw_text_t(width / 3 + 200.f, 100.0f, 100.0f, 35.0f));
	portField->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	rtd::Handler2D::Get().InsertElement(portField, "serverConnect");
	portField->SetName("portBuffer");
	portField->GetText()->SetText("Input PORT");
	

	rtd::Button* connectButton = new rtd::Button("StartButton.png", draw_t(width / 2 - 150.f, height - height / 3, 300.0f, 100.0f));
	rtd::Handler2D::Get().InsertElement(connectButton, "serverConnect");
	connectButton->SetName("connectButton");
#endif
}

void SetupMainMenuScreen(Window* pWindow)
{
#if RENDER_IMGUI == 0
	// Setup main menu scene.
	const unsigned int width = pWindow->GetWidth(), height = pWindow->GetHeight();
	const float buttonWidth = 350.0f, buttonHeight = 75.f;
	const float buttonsLeft = width / 4 - buttonWidth / 2;


	// Adds text to the menu screen.
	rtd::Text* welcomeText = new rtd::Text("Welcome To Homehearth!", draw_text_t(575.0f, 50.0f, 300.0f, 100.0f));
	rtd::Handler2D::Get().InsertElement(welcomeText, "mainMenu");
	welcomeText->SetName("welcome_text");
	
	// Adds text to the menu screen.
	rtd::Text* gameInfoText = new rtd::Text("In this game you will face against very dangerous foes while defending the righteous village from its dark fate! Take up arms and fight your way to victory champion! Join our discord and twitter to get official news about the new upcoming technological wonder game! Sign up for RTX exclusive version at our website!", draw_text_t(550.0f, 0.0f, 350.0f, 550.0f));
	rtd::Handler2D::Get().InsertElement(gameInfoText, "mainMenu");
	gameInfoText->SetName("gameInfoText");

	// Adds a button and names it joinButton.
	rtd::Button* joinButton = new rtd::Button("StartButton.png", draw_t(buttonsLeft, 100.0f, buttonWidth, buttonHeight));
	rtd::Handler2D::Get().InsertElement(joinButton, "mainMenu");
	joinButton->SetName("joinButton");

	rtd::TextField* lobbyField = new rtd::TextField(draw_text_t(buttonsLeft + buttonWidth + 10.f, 117.5f, 100.0f, 35.0f));
	// Adds a border around the element and sets the color to black.
	lobbyField->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	rtd::Handler2D::Get().InsertElement(lobbyField, "mainMenu");
	lobbyField->SetName("lobbyBuffer");
	lobbyField->GetText()->SetText("Input Lobby ID");

	
	// Adds a button and names it start game button.
	rtd::Button* hostButton = new rtd::Button("StartButton.png", draw_t(buttonsLeft, 185.0f, buttonWidth, buttonHeight));
	rtd::Handler2D::Get().InsertElement(hostButton, "mainMenu");
	hostButton->SetName("hostButton");
	// Adds a border around the button and sets the color to black.


	// Adds a button and names it exit game button.
	rtd::Button* exitGameButton = new rtd::Button("demo_exit_button.png", draw_t(buttonsLeft, 325.0f, buttonWidth, buttonHeight));
	rtd::Handler2D::Get().InsertElement(exitGameButton, "mainMenu");
	exitGameButton->SetName("exitGameButton");
	// Adds a border around the button and sets the color to black.
	exitGameButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
#endif
}

void SetupInGameScreen()
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
	rtd::Handler2D::Get().InsertElement(heart1, "inGame");

	rtd::Picture* heart2 = new rtd::Picture(texture1, draw_t(174.0f, 10.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart2, "inGame");

	rtd::Picture* heart3 = new rtd::Picture(texture1, draw_t(258.0f, 10.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart3, "inGame");

	rtd::Text* youText = new rtd::Text("You:", draw_text_t(5.0f, 30.0f, 60.0f, 20.0f));
	rtd::Handler2D::Get().InsertElement(youText, "inGame");

	// Player 2
	rtd::Picture* heart4 = new rtd::Picture(texture1, draw_t(90.0f, 72.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart4, "inGame");

	rtd::Picture* heart5 = new rtd::Picture(texture1, draw_t(174.0f, 72.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart5, "inGame");

	rtd::Picture* heart6 = new rtd::Picture(texture1, draw_t(258.0f, 72.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart6, "inGame");

	rtd::Text* friendText = new rtd::Text("Friend:", draw_text_t(5.0f, 95.0f, 84.0f, 20.0f));
	rtd::Handler2D::Get().InsertElement(friendText, "inGame");

	//Timer text
	const std::string& timerText = "1:20";
	rtd::Text* timer = new rtd::Text(timerText, draw_text_t(436.0f, 24.0f, 96.0f, 24.0f));
	rtd::Handler2D::Get().InsertElement(timer, "inGame");

	// Attacks
	rtd::Text* attacksText = new rtd::Text("Attacks!", draw_text_t(24.0f, 412.0f, 96.0f, 24.0f));
	rtd::Handler2D::Get().InsertElement(attacksText, "inGame");

	rtd::Picture* attack1 = new rtd::Picture(texture2, draw_t(24.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(attack1, "inGame");

	rtd::Picture* attack2 = new rtd::Picture(texture3, draw_t(98.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(attack2, "inGame");

	rtd::Picture* attack3 = new rtd::Picture(texture4, draw_t(172.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(attack3, "inGame");
	attack3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	attack2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	attack1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));

	// Builds
	rtd::Text* buildText = new rtd::Text("Builds!", draw_text_t(700.0f, 412.0f, 96.0f, 24.0f));
	rtd::Handler2D::Get().InsertElement(buildText, "inGame");

	rtd::Picture* build1 = new rtd::Picture(texture2, draw_t(700.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(build1, "inGame");

	rtd::Picture* build2 = new rtd::Picture(texture3, draw_t(774.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(build2, "inGame");

	rtd::Picture* build3 = new rtd::Picture(texture4, draw_t(848.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(build3, "inGame");
	build3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	build2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	build1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
#endif
}

void SetupInLobbyScreen()
{
#if RENDER_IMGUI == 0
	const std::string& warriorString = "Warrior\nThe Warrior specializes in destroying noobs.";
	const std::string& mageString = "Mage\nThis weak character is good for nothing please choose the warrior instead.";

	rtd::Canvas* backgroundCanvas = new rtd::Canvas(D2D1::ColorF(.2f, .2f, .2f), draw_t(0.0f, 0.0f, 3000.0f, 3000.0f));
	rtd::Handler2D::Get().InsertElement(backgroundCanvas);

	rtd::Button* startGameButton = new rtd::Button("StartButton.png", draw_t(625.0f, 420.0f, 250.0f, 100.0f), true);
	rtd::Handler2D::Get().InsertElement(startGameButton);
	startGameButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));

	rtd::Canvas* textCanvas = new rtd::Canvas(D2D1::ColorF(0.0f, 0.0f, 0.0f), draw_t(580.0f, 10.0f, 350.0f, 400.0f));
	rtd::Handler2D::Get().InsertElement(textCanvas);

	const std::string& dt = "##--Description--##";
	rtd::Text* descText = new rtd::Text(dt, draw_text_t(530.0f, 20.0f, dt.length() * 24.0f, 24));
	rtd::Handler2D::Get().InsertElement(descText);

	rtd::Text* warriorText = new rtd::Text(warriorString, draw_text_t(580.0f, 30.0f, 350.0f, 370.0f));
	rtd::Handler2D::Get().InsertElement(warriorText);
	warriorText->SetName("warrior_text");

	// Init mage text but set it to no render.
	rtd::Text* mageText = new rtd::Text(mageString, draw_text_t(580.0f, 30.0f, 350.0f, 370.0f));
	rtd::Handler2D::Get().InsertElement(mageText);
	mageText->SetVisibility(false);
	mageText->SetName("mage_text");

	rtd::Canvas* lobbyIdCanvas = new rtd::Canvas(D2D1::ColorF(0.0f, 0.0f, 0.0f), draw_t(30.0f, 420.0f, 275.0f, 100.0f));
	rtd::Handler2D::Get().InsertElement(lobbyIdCanvas);

	const std::string& lobbyString = "Lobby ID: XYZW";
	rtd::Text* lobbyIdText = new rtd::Text(lobbyString, draw_text_t(0.0f, 460.0f, lobbyString.length() * 24.0f, 24.0f));
	rtd::Handler2D::Get().InsertElement(lobbyIdText);

	rtd::Button* mageButton = new rtd::Button("mageIconDemo.png", draw_t(350.0f, 454.0f, 32.0f, 32.0f));
	rtd::Handler2D::Get().InsertElement(mageButton);
	mageButton->SetName("mage_button");

	rtd::Button* warriorButton = new rtd::Button("warriorIconDemo.png", draw_t(392.0f, 454.0f, 32.0f, 32.0f));
	rtd::Handler2D::Get().InsertElement(warriorButton);
	warriorButton->SetName("warrior_button");

	// Player slots
	for (int i = 0; i < 2; i++)
	{
		const std::string& playerString = "Player " + std::to_string(i + 1);
		rtd::Text* playerText = new rtd::Text(playerString, draw_text_t(0.0f, (i + 1) * 100.0f + ((i + 1) * 25.0f), playerString.length() * 24.0f, 64.0f));
		rtd::Canvas* playerCanvas = new rtd::Canvas(D2D1::ColorF(0.7f, 0.5f, 0.2f), draw_t(25.0f, (i + 1) * 100.0f + ((i + 1) * 25.0f), 300.0f, 64.0f));
		rtd::Handler2D::Get().InsertElement(playerCanvas);
		rtd::Handler2D::Get().InsertElement(playerText);
	}

	rtd::Picture* player1Symbol = new rtd::Picture("warriorIconDemo.png", draw_t(350.0f, 125.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(player1Symbol);
	player1Symbol->SetName("player1_symbol");

	rtd::Picture* host = new rtd::Picture("demoHost.png", draw_t(424.0f, 125.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(host);
	host->SetName("host_symbol");

	rtd::Picture* player2Symbol = new rtd::Picture("warriorIconDemo.png", draw_t(350.0f, 250.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(player2Symbol);
	player2Symbol->SetName("player2_symbol");

	rtd::Text* homehearthText = new rtd::Text("Homehearth", draw_text_t(25.0f, 25.0f, 200.0f, 50.0f));
	rtd::Handler2D::Get().InsertElement(homehearthText);
#endif
}

void SetupOptionsScreen()
{
	
}