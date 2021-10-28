#include "Game.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_gameID = -1;
	this->m_isLeavingLobby = false;
	this->m_predictionThreshhold = 0.001f;
}

Game::~Game()
{
	if (m_client.IsConnected())
	{
		m_client.Disconnect();
	}
}

void Game::UpdateNetwork(float deltaTime)
{
	static float pingCheck = 0.f;
	const float TARGET_PING_TIME = 5.0f;
	if (m_client.IsConnected())
	{
		m_client.Update();

		pingCheck += deltaTime;

		if (pingCheck > TARGET_PING_TIME)
		{
			this->PingServer();
			pingCheck -= TARGET_PING_TIME;
		}

		if (GetCurrentScene() == &GetScene("Game") && !m_isLeavingLobby)
		{
			if (GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
			{
				
				message<GameMsg> msg;
				msg.header.id = GameMsg::Game_PlayerInput;

				InputState input;
				input.axisX = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
				input.axisY = InputSystem::Get().GetAxis(Axis::VERTICAL);

				input.tick = 0; // todo

				msg << this->m_localPID << m_gameID << input;

				m_client.Send(msg);
			}

			if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
			{
				message<GameMsg> msg;
				msg.header.id = GameMsg::Game_PlayerAttack;
				Ray_t ray = InputSystem::Get().GetMouseRay();
				msg << ray << this->m_localPID << m_gameID;
				
				m_client.Send(msg);
			}
		}
	}
}

bool Game::OnStartup()
{
	// Scene logic
	sceneHelp::CreateLobbyScene(*this);
	rtd::Handler2D::Get().SetVisibilityAll(false);
	sceneHelp::CreateGameScene(*this);
	sceneHelp::CreateMainMenuScene(*this);



	// Set Current Scene
	SetScene("MainMenu");
	
	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
	static float pingCheck = 0.f;

#if RENDER_IMGUI == 0

	switch (m_internalState)
	{
	case 0:
	{
		// IN MENU!

		rtd::TextField* port_text = GET_ELEMENT("portBuffer", rtd::TextField);
		rtd::TextField* ip_text = GET_ELEMENT("ipBuffer", rtd::TextField);

		rtd::Button* connect_button = GET_ELEMENT("connectButton", rtd::Button);
		if (connect_button)
		{
			if (connect_button->IsClicked())
			{
				if (port_text && ip_text)
				{
					m_ipBuffer = ip_text->RawGetBuffer();
					m_portBuffer = port_text->RawGetBuffer();
					if (m_ipBuffer && m_portBuffer)
					{
						if (m_client.Connect(m_ipBuffer->c_str(), std::stoi(*m_portBuffer)))
						{
							rtd::Handler2D::Get().SetVisibilityAll(false);
							m_ipBuffer = nullptr;
							m_portBuffer = nullptr;
							m_internalState = 2;
							break;
						}
					}
					else
					{
						m_ipBuffer = nullptr;
						m_portBuffer = nullptr;
					}
				}
			}
		}
		else
		{
			//sceneHelp::SetupLobbyJoinScreen(GetWindow(), 1);
		}

		rtd::Button* exit_button = GET_ELEMENT("exitGameButton", rtd::Button);
		if (exit_button)
		{
			if (exit_button->IsClicked())
			{
				this->Shutdown();
			}
		}

		rtd::Button* start_button = GET_ELEMENT("startGameButton", rtd::Button);
		if (start_button)
		{
			if (start_button->IsClicked())
			{
				rtd::Handler2D::Get().SetVisibilityAll(false);
				ip_text->SetVisibility(true);
				port_text->SetVisibility(true);
				if (connect_button)
				{
					connect_button->SetVisibility(true);
				}
			}
		}
		break;
	}
	case 1:
	{
		// IN LOBBY STATE!

		rtd::Button* ready_button = GET_ELEMENT("readyButton", rtd::Button);
		if (ready_button)
		{
			if (ready_button->IsClicked())
			{
				// Ready for game start.
				LOG_INFO("You have pressed [READY] Button.");
				this->SendStartGame();
			}
		}

		rtd::Button* leave_button = GET_ELEMENT("exitToMainButton", rtd::Button);
		if (leave_button)
		{
			// Leave lobby.
			if (leave_button->IsClicked())
			{
				m_isLeavingLobby = true;
				message<GameMsg> msg;
				msg.header.id = GameMsg::Lobby_Leave;
				msg << m_localPID << m_gameID;
				m_client.Send(msg);
				rtd::Handler2D::Get().Cleanup();
				sceneHelp::SetupLobbyJoinScreen(GetWindow(), 1);
				m_internalState = 2;
			}
		}

		break;
	}
	default:
	{
		break;
	}
	case 2:
	{
		// JOIN A LOBBY STATE!

		if (m_client.IsConnected())
		{
			rtd::TextField* lobby_text = GET_ELEMENT("lobbyBuffer", rtd::TextField);
			if (lobby_text)
			{
				lobby_text->SetVisibility(true);
			}

			// Either join or host lobby.
			rtd::Button* host_lobby_button = GET_ELEMENT("hostLobby", rtd::Button);
			if (host_lobby_button && lobby_text)
			{
				host_lobby_button->SetVisibility(true);
				std::string* lobby = nullptr;
				if (host_lobby_button->IsClicked())
				{
					lobby = lobby_text->RawGetBuffer();
					if (lobby->size() <= 0)
					{
						rtd::Handler2D::Get().Cleanup();
						this->CreateLobby();
						m_internalState = 1;
						m_lobbyBuffer = nullptr;
					}
					else
					{
						int port = std::stoi(*lobby);
						this->JoinLobby(port);
						m_internalState = 1;
						m_lobbyBuffer = nullptr;
						rtd::Handler2D::Get().Cleanup();
					}
				}
			}
		}
		break;
	}
	}
#endif

	IMGUI(
		ImGui::Begin("Network");

	if (m_client.IsConnected())
	{
		ImGui::Text(std::string("Local Client ID: " + std::to_string(m_localPID)).c_str());

		if (m_client.m_latency > 0)
		{
			ImGui::Text(std::string("Latency: " + std::to_string(m_client.m_latency) + "ms").c_str());
		}
		else
		{
			ImGui::Text(std::string("Latency: <1 ms").c_str());
		}

		if (GetCurrentScene() != &GetScene("Game"))
		{
			if (ImGui::Button("Create Lobby"))
			{
				this->CreateLobby();
			}
			static uint32_t lobbyID = 0;
			ImGui::InputInt("LobbyID", (int*)&lobbyID);
			ImGui::SameLine();

			if (ImGui::Button("Join"))
			{
				this->JoinLobby(lobbyID);
			}
		}
		else
		{
			ImGui::Text(std::string("Game ID: " + std::to_string(m_gameID)).c_str());

			if (m_isLeavingLobby)
			{
				ImGui::BeginDisabled();
				ImGui::Button("Leave Game");
				ImGui::EndDisabled();

			}
			else if (ImGui::Button("Leave Game"))
			{
				m_isLeavingLobby = true;
				message<GameMsg> msg;
				msg.header.id = GameMsg::Lobby_Leave;
				msg << m_localPID << m_gameID;
				m_client.Send(msg);
			}

		}
		if (ImGui::Button("Disconnect"))
		{
			this->m_client.Disconnect();
		}
	}
	else
	{
		static char buffer[IPV6_ADDRSTRLEN] = "127.0.0.1";
		ImGui::InputText("IP", buffer, IPV6_ADDRSTRLEN);
		static uint16_t port = 4950;
		ImGui::InputInt("Port", (int*)&port);
		if (ImGui::Button("Connect"))
		{
			m_client.Connect(buffer, port);
		}
	}
	ImGui::End();
	);

	
	if (GetCurrentScene() == &GetScene("Game") && GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
	{
		GetCurrentScene()->ForEachComponent<comp::Transform, comp::Velocity, comp::Player, comp::Tag<TagType::LOCAL_PLAYER>>([&]
		(comp::Transform& t, comp::Velocity& v, comp::Player& p, comp::Tag<TagType::LOCAL_PLAYER>& tag)
			{
				int x = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
				int z = InputSystem::Get().GetAxis(Axis::VERTICAL);
			

				v.vel = sm::Vector3(x, 0, z) * p.runSpeed;
				t.position += v.vel * deltaTime;
				
				predictedPositions.push_back(t);
			

				//LOG_INFO("Predicted size: %llu", predictedPositions.size());
				//if (sm::Vector3::Distance(t.position, test.position) > m_predictionThreshhold)
				//{
				//	t.position.x = test.position.x;
				//	t.position.z = test.position.z;
				//}
			}
		);
	}
}


void Game::OnShutdown()
{

}


void Game::CheckIncoming(message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Client_Accepted:
	{
		LOG_INFO("You are validated!");
		break;
	}
	case GameMsg::Server_GetPing:
	{
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		m_client.m_latency = int(std::chrono::duration<double>(timeNow - this->m_timeThen).count() * 1000);
		break;
	}
	case GameMsg::Server_AssignID:
	{
		msg >> this->m_localPID;

		LOG_INFO("YOUR ID IS: %lu", this->m_localPID);
		break;
	}
	case GameMsg::Game_Snapshot:
	{
		uint32_t currentTick;
		msg >> currentTick;
		uint32_t count;
		msg >> count;

		std::unordered_map<uint32_t, comp::Transform> transforms;

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			comp::Transform t;
			msg >> entityID >> t;
			transforms[entityID] = t;
		}
		// TODO MAKE BETTER
		// Update entities with new transforms
		GetScene("Game").ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& n, comp::Transform& t)
			{
				if (transforms.find(n.id) != transforms.end())
				{
					t = transforms.at(n.id);
					if (n.id == m_localPID)
					{
						test = transforms.at(n.id);
						predictedPositions.clear();
					}
				}
			});

		break;
	}
	case GameMsg::Game_AddEntity:
	{
		uint32_t count; // Could be more than one Entity
		msg >> count;
		LOG_INFO("Received %u entities", count);

		for (uint32_t i = 0; i < count; i++)
		{
			Entity e = CreateEntityFromMessage(msg);

			LOG_INFO("Added entity %u", e.GetComponent<comp::Network>()->id);

			if (e.GetComponent<comp::Network>()->id == m_localPID)
			{
				LOG_INFO("This player added");
				e.AddComponent<comp::Tag<TagType::LOCAL_PLAYER>>();

				GetScene("Game").ForEachComponent<comp::Tag<TagType::CAMERA>>([&](Entity entt, comp::Tag<TagType::CAMERA>& t)
					{
						comp::Camera3D* c = entt.GetComponent<comp::Camera3D>();
						if (c)
						{
							c->camera.SetFollowEntity(e);
						}
					});
			}
		}

		break;
	}
	case GameMsg::Game_RemoveEntity:
	{
		uint32_t count;
		msg >> count;
		std::vector<uint32_t> ids(count);
		for (uint32_t i = 0; i < count; i++)
		{
			msg >> ids[i];
		}

		GetScene("Game").ForEachComponent<comp::Network>([&](Entity& e, comp::Network& net)
			{
				if (std::find(ids.begin(), ids.end(), net.id) != ids.end())
				{
					LOG_INFO("Removed Entity %u", net.id);
					e.Destroy();
				}
			});

		break;
	}
	//case GameMsg::Game_AddEnemy:
	//{
	//	uint32_t count; // Could be more than one enemy
	//	msg >> count;
	//	for (uint32_t i = 0; i < count; i++)
	//	{
	//		LOG_INFO("A wild enemy has appeared!");
	//		Entity e = m_demoScene->CreateEnemy();
	//	}

	//	break;
	//}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
		sceneHelp::SetupInLobbyScreen();
		m_internalState = 1;
		SetScene("Lobby");
		LOG_INFO("You are now in lobby: %lu", m_gameID);


		// Update the lobby ID text
		rtd::Text* lobbyIdText = GET_ELEMENT("LobbyIdText", rtd::Text);
		if (lobbyIdText)
		{
			lobbyIdText->SetText("Lobby ID: " + std::to_string(m_gameID));
		}
		break;
	}
	case GameMsg::Lobby_Invalid:
	{
		std::string err;
		msg >> err;
		SetScene("MainMenu");
		m_internalState = 2;
		LOG_WARNING("Request denied: %s", err.c_str());
		break;
	}
	case GameMsg::Lobby_AcceptedLeave:
	{
		LOG_WARNING("Left Lobby %u", m_gameID);
		m_isLeavingLobby = false;
		m_gameID = -1;
		m_internalState = 2;
		SetScene("MainMenu");
		rtd::Handler2D::Get().SetVisibilityAll(false);

		break;
	}
	case GameMsg::Game_Start:
	{
		m_internalState = 3;
		rtd::Handler2D::Get().Cleanup();
		SetScene("Game");
		break;
	}
	case GameMsg::Lobby_Update:
	{
		uint32_t nrOfPlayers = 0;
		uint32_t player = -1;
		uint8_t state = 0;
		msg >> state >> player >> nrOfPlayers;

		rtd::Text* player2Text = GET_ELEMENT("player2text", rtd::Text);
		rtd::Canvas* player2Canvas = GET_ELEMENT("player2canvas", rtd::Canvas);
		rtd::Picture* player2Symbol = GET_ELEMENT("player2_symbol", rtd::Picture);

		rtd::Text* player1Text = GET_ELEMENT("player1text", rtd::Text);
		rtd::Canvas* player1Canvas = GET_ELEMENT("player1canvas", rtd::Canvas);
		rtd::Picture* player1Symbol = GET_ELEMENT("player1_symbol", rtd::Picture);

		if (!player2Text || !player2Canvas || !player2Symbol || !player1Text || !player1Canvas || !player1Symbol)
			break;

		if (player == 1)
		{
			// Hide or show player1 depending on state.
			if (state == 2)
			{
				player1Text->SetVisibility(false);
				player1Canvas->SetVisibility(false);
				player1Symbol->SetVisibility(false);
			}
			else if (state == 1)
			{
				player1Text->SetVisibility(true);
				player1Canvas->SetVisibility(true);
				player1Symbol->SetVisibility(true);
			}
		}
		else if (player == 2)
		{
			// Hide or show player2 depending on state.
			if (state == 2)
			{
				player2Text->SetVisibility(false);
				player2Canvas->SetVisibility(false);
				player2Symbol->SetVisibility(false);
			}
			else if (state == 1)
			{
				player2Text->SetVisibility(true);
				player2Canvas->SetVisibility(true);
				player2Symbol->SetVisibility(true);
			}
		}

		// Set all visible.
		if (nrOfPlayers == 2)
		{
			player1Text->SetVisibility(true);
			player1Canvas->SetVisibility(true);
			player1Symbol->SetVisibility(true);
			player2Text->SetVisibility(true);
			player2Canvas->SetVisibility(true);
			player2Symbol->SetVisibility(true);
		}

		break;
	}
	}
}

void Game::PingServer()
{
	message<GameMsg> msg = {};
	msg.header.id = GameMsg::Server_GetPing;
	msg << this->m_localPID;

	this->m_timeThen = std::chrono::system_clock::now();
	m_client.Send(msg);
}

void Game::JoinLobby(uint32_t lobbyID)
{
	if (m_gameID == (uint32_t)-1)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Join;
		msg << this->m_localPID << lobbyID;
		m_client.Send(msg);
	}
	else
	{
		LOG_WARNING("Request denied: You are already in a lobby");
	}
}

void Game::CreateLobby()
{
	if (m_gameID == (uint32_t)-1)
	{
		message<GameMsg> msg;

		msg.header.id = GameMsg::Lobby_Create;
		msg << this->m_localPID;
		m_client.Send(msg);
	}
	else
	{
		LOG_WARNING("Request denied: You are already in a lobby");
	}
}

void Game::OnClientDisconnect()
{
	this->m_gameID = -1;
	this->m_localPID = -1;

	// remove all network entities
	GetScene("Game").ForEachComponent<comp::Network>([](Entity& e, comp::Network& net)
		{
			LOG_INFO("Removed entity %u on disconnect", net.id);
			e.Destroy();
		}
	);

	SetScene("MainMenu");

	LOG_INFO("Disconnected from server!");
}

void Game::SendStartGame()
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_PlayerReady;
	msg << m_localPID << m_gameID;
	m_client.Send(msg);
}

Entity Game::CreateEntityFromMessage(message<GameMsg>& msg)
{
	Entity e = GetScene("Game").CreateEntity();
	uint32_t bits;
	msg >> bits;
	std::bitset<ecs::Component::COMPONENT_MAX> compSet(bits);

	for (int i = ecs::Component::COMPONENT_COUNT - 1; i >= 0; i--)
	{
		if (compSet.test(i))
		{
			switch (i)
			{
			case ecs::Component::NETWORK:
			{
				comp::Network n;
				msg >> n;
				*e.AddComponent<comp::Network>() = n;
				break;
			}
			case ecs::Component::TRANSFORM:
			{
				comp::Transform t;
				msg >> t;
				*e.AddComponent<comp::Transform>() = t;
				break;
			}
			case ecs::Component::VELOCITY:
			{
				comp::Velocity v;
				msg >> v;
				*e.AddComponent<comp::Velocity>() = v;
				break;
			}
			case ecs::Component::MESH_NAME:
			{
				std::string name;
				msg >> name;
				e.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>(name);
				break;
			}
			case ecs::Component::BOUNDING_ORIENTED_BOX:
			{
				comp::BoundingOrientedBox box;
				msg >> box;
				*e.AddComponent<comp::BoundingOrientedBox>() = box;
				break;
			}
			case ecs::Component::BOUNDING_SPHERE:
			{
				comp::BoundingSphere s;
				msg >> s;
				*e.AddComponent<comp::BoundingSphere>() = s;
				break;
			}
			case ecs::Component::LIGHT:
			{
				comp::Light l;
				msg >> l;
				*e.AddComponent<comp::Light>() = l;
				break;
			}
			case ecs::Component::PLAYER:
			{
				comp::Player p;
				msg >> p;
				*e.AddComponent<comp::Player>() = p;
				break;
			}
			default:
				LOG_WARNING("Retrieved unimplemented component %u", i)
					break;
			}
		}
	}


	return e;
}

