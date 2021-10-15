#include "DemoScene.h"

void SetupMainMenuScreen();
void SetupInGameScreen();
void SetupInLobbyScreen();
void SetupOptionsScreen();

DemoScene::DemoScene(Engine& engine, Client& client, uint32_t* playerID, uint32_t* gameID)
	: SceneBuilder(engine)
	, m_gameID(gameID)
	, m_playerID(playerID)
	, m_client(client)
{
	m_engine = &engine;
	//Initialize player entity
	for(int i = 0; i < 3; i++)
		m_player = CreatePlayerEntity();

	SetUpCamera();


	// Define what scene does on update
	m_scene.on<ESceneUpdate>([&](const ESceneUpdate& e, Scene& scene)
		{
			if(CollisionSystem::Get().IsColliding(m_chest, m_player))
			{
				LOG_INFO("Player is Colliding with box...");
			}
			//System responding to user input
			GameSystems::MRayIntersectBoxSystem(scene);

			/*
				Logic for buttons.
			*/

			/*
			if (rtd::Handler2D::Get().GetElement<rtd::Button>("startGameButton")->IsClicked())
			{
				std::cout << "You started the game! :D\n";
			}

			if (rtd::Handler2D::Get().GetElement<rtd::Button>("exitGameButton")->IsClicked())
			{
				std::cout << "You exit the game! :)\n";
			}
			*/

			int ver = InputSystem::Get().GetAxis(Axis::VERTICAL);
			int hor = InputSystem::Get().GetAxis(Axis::HORIZONTAL);

			if (scene.m_currentCamera.get()->GetCameraType() == CAMERATYPE::PLAY)
			{
				m_player.GetComponent<comp::Velocity>()->vel.z = ver * m_player.GetComponent<comp::Player>()->runSpeed;
				m_player.GetComponent<comp::Velocity>()->vel.x = hor * m_player.GetComponent<comp::Player>()->runSpeed;

				// Updates the position based on input from player
				Systems::MovementSystem(scene, e.dt);
				//System responding to user input
				GameSystems::UserInputSystem(scene, client);

				
				//m_player.GetComponent<comp::Velocity>()->vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * m_player.GetComponent<comp::Player>()->runSpeed;
				//m_player.GetComponent<comp::Velocity>()->vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * m_player.GetComponent<comp::Player>()->runSpeed;
			}

			if (m_client.IsConnected() && *m_gameID != UINT32_MAX)
			{
				// send updated player position
				network::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_Update;
				comp::Transform t = *m_player.GetComponent<comp::Transform>();
				msg << t << *m_playerID << *m_gameID;
				m_client.Send(msg);
			}
		
			GameSystems::MRayIntersectBoxSystem(scene);

			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(scene);
			GameSystems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(scene);
		});

	//On collision event add entitys as pair in the collision system
	m_scene.on<ESceneCollision>([&](const ESceneCollision& e, Scene& scene)
		{
			if(e.obj1 != e.obj2)
				CollisionSystem::Get().AddPair(e.obj1, e.obj2);
		});

	SetupMainMenuScreen();
	//SetupInGameScreen();
}

void DemoScene::SetUpCamera()
{
	m_gameCamera.Initialize(sm::Vector3(0, 0, -10), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::PLAY);
	m_scene.m_currentCamera = std::make_unique<Camera>(m_gameCamera);

#ifdef _DEBUG
	m_debugCamera.Initialize(sm::Vector3(0, 0, -20), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2((float)m_engine->GetWindow()->GetWidth(), (float)m_engine->GetWindow()->GetHeight()), CAMERATYPE::DEBUG);
	//m_scene.m_currentCamera = std::make_unique<Camera>(m_debugCamera);

#endif // DEBUG
	InputSystem::Get().SetCamera(m_scene.m_currentCamera.get());
}

void DemoScene::CameraUpdate(float deltaTime)
{
	m_scene.m_currentCamera->Update(deltaTime);
	CAMERATYPE test = m_scene.m_currentCamera->GetCameraType();

#ifdef _DEBUG
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Space, KeyState::RELEASED))
	{
		if (m_scene.m_currentCamera->GetCameraType() == CAMERATYPE::DEBUG)
		{
			m_oldDebugCameraPosition = m_scene.m_currentCamera.get()->GetPosition();
			*m_scene.m_currentCamera = m_gameCamera;
			m_scene.m_currentCamera.get()->SetPosition(m_oldGameCameraPosition);

			LOG_INFO("Game Camera selected");
		}
		else if (m_scene.m_currentCamera->GetCameraType() == CAMERATYPE::PLAY)
		{
			m_oldGameCameraPosition = m_scene.m_currentCamera.get()->GetPosition();
			*m_scene.m_currentCamera = m_debugCamera;
			m_scene.m_currentCamera.get()->SetPosition(m_oldDebugCameraPosition);

			LOG_INFO("Debugg Camera selected");
		}
	}
#endif // DEBUG
}

Entity DemoScene::CreatePlayerEntity()
{
	//Player
	Entity playerEntity = m_scene.CreateEntity();
	playerEntity.AddComponent<comp::Transform>();
	
	comp::BoundingOrientedBox* playerObb = playerEntity.AddComponent<comp::BoundingOrientedBox>();
	playerObb->Extents = sm::Vector3{ 1.f,1.f,1.f };

	comp::Renderable* renderable = playerEntity.AddComponent<comp::Renderable>();
	renderable->model = ResourceManager::Get().GetResource<RModel>("Cube.obj");

	playerEntity.AddComponent<comp::Player>()->runSpeed = 10.f;

	comp::Velocity* playeerVelocity = playerEntity.AddComponent<comp::Velocity>();
	m_gameCamera.SetFollowVelocity(playeerVelocity);


	//Second object
	this->m_chest = m_scene.CreateEntity();
	comp::Transform* transform = m_chest.AddComponent<comp::Transform>();
	transform->position.z = 5;

	comp::Velocity* chestVelocity = m_chest.AddComponent<comp::Velocity>();
	comp::BoundingSphere* obb = m_chest.AddComponent<comp::BoundingSphere>();
	obb->Center = transform->position;
	obb->Radius = 2.0f;
	comp::Renderable* renderable2 = m_chest.AddComponent<comp::Renderable>();

	renderable2->model = ResourceManager::Get().GetResource<RModel>("Chest.obj");

	return playerEntity;
}

void SetupMainMenuScreen()
{
	// Setup main menu scene.

	// Adds text to the menu screen.
	rtd::Text* welcomeText = new rtd::Text("Welcome To Homehearth!", draw_text_t(575.0f, 50.0f, 300.0f, 100.0f));
	rtd::Handler2D::Get().InsertElement(welcomeText);
	welcomeText->SetName("welcome_text");

	// Adds text to the menu screen.
	rtd::Text* gameInfoText = new rtd::Text("In this game you will face against very dangerous foes while defending the righteous village from its dark fate! Take up arms and fight your way to victory champion! Join our discord and twitter to get official news about the new upcoming technological wonder game! Sign up for RTX exclusive version at our website!", draw_text_t(550.0f, 0.0f, 350.0f, 550.0f));
	rtd::Handler2D::Get().InsertElement(gameInfoText);
	gameInfoText->SetName("gameInfoText");

	// Adds a button and names it start game button.
	rtd::Button* startGameButton = new rtd::Button("demo_start_game_button.png", draw_t(100.0f, 100.0f, 350.0f, 150.0f));
	rtd::Handler2D::Get().InsertElement(startGameButton);
	startGameButton->SetName("startGameButton");
	// Adds a border around the button and sets the color to black.
	startGameButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));

	// Adds a button and names it exit game button.
	rtd::Button* exitGameButton = new rtd::Button("demo_exit_button.png", draw_t(100.0f, 325.0f, 350.0f, 150.0f));
	rtd::Handler2D::Get().InsertElement(exitGameButton);
	exitGameButton->SetName("exitGameButton");
	// Adds a border around the button and sets the color to black.
	exitGameButton->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));


}

void SetupInGameScreen()
{
	// Temp textures
	const std::string& texture1 = "Leafs3.png";
	const std::string& texture2 = "LampFlowerLeafs.png";
	const std::string& texture3 = "LeafsDark.png";
	const std::string& texture4 = "Light.png";

	// Player 1
	// Temp 'heart' displayed as leaf
	rtd::Picture* heart1 = new rtd::Picture(texture1, draw_t(90.0f, 10.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart1);

	rtd::Picture* heart2 = new rtd::Picture(texture1, draw_t(174.0f, 10.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart2);

	rtd::Picture* heart3 = new rtd::Picture(texture1, draw_t(258.0f, 10.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart3);

	rtd::Text* youText = new rtd::Text("You:", draw_text_t(5.0f, 30.0f, 60.0f, 20.0f));
	rtd::Handler2D::Get().InsertElement(youText);

	// Player 2
	rtd::Picture* heart4 = new rtd::Picture(texture1, draw_t(90.0f, 72.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart4);

	rtd::Picture* heart5 = new rtd::Picture(texture1, draw_t(174.0f, 72.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart5);

	rtd::Picture* heart6 = new rtd::Picture(texture1, draw_t(258.0f, 72.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(heart6);

	rtd::Text* friendText = new rtd::Text("Friend:", draw_text_t(5.0f, 95.0f, 84.0f, 20.0f));
	rtd::Handler2D::Get().InsertElement(friendText);

	//Timer text
	const std::string& timerText = "1:20";
	rtd::Text* timer = new rtd::Text(timerText, draw_text_t(436.0f, 24.0f, 96.0f, 24.0f));
	rtd::Handler2D::Get().InsertElement(timer);

	// Attacks
	rtd::Text* attacksText = new rtd::Text("Attacks!", draw_text_t(24.0f, 412.0f, 96.0f, 24.0f));
	rtd::Handler2D::Get().InsertElement(attacksText);

	rtd::Picture* attack1 = new rtd::Picture(texture2, draw_t(24.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(attack1);

	rtd::Picture* attack2 = new rtd::Picture(texture3, draw_t(98.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(attack2);

	rtd::Picture* attack3 = new rtd::Picture(texture4, draw_t(172.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(attack3);
	attack3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	attack2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	attack1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));

	// Builds
	rtd::Text* buildText = new rtd::Text("Builds!", draw_text_t(700.0f, 412.0f, 96.0f, 24.0f));
	rtd::Handler2D::Get().InsertElement(buildText);

	rtd::Picture* build1 = new rtd::Picture(texture2, draw_t(700.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(build1);

	rtd::Picture* build2 = new rtd::Picture(texture3, draw_t(774.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(build2);

	rtd::Picture* build3 = new rtd::Picture(texture4, draw_t(848.f, 448.0f, 64.0f, 64.0f));
	rtd::Handler2D::Get().InsertElement(build3);
	build3->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	build2->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
	build1->GetBorder()->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
}

void SetupInLobbyScreen()
{
}

void SetupOptionsScreen()
{
}
