#include "Game.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_gameID = -1;
	this->m_isLeavingLobby = false;
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

		// TODO MAKE THIS BETTER
		if (m_gameID != UINT32_MAX && GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY && !m_isLeavingLobby)
		{
			message<GameMsg> msg;
			msg.header.id = GameMsg::Game_PlayerInput;
			int8_t x = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
			int8_t y = InputSystem::Get().GetAxis(Axis::VERTICAL);

			msg << this->m_localPID << m_gameID << x << y;

			m_client.Send(msg);
		}
	}
}

bool Game::OnStartup()
{	
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
		// IN MENU AND CONNECT TO LOBBY STATE!

		rtd::TextField* port_text = GET_ELEMENT("portBuffer", rtd::TextField);
		rtd::TextField* ip_text = GET_ELEMENT("ipBuffer", rtd::TextField);
		if (ip_text && port_text)
		{
			ip_text->GetBuffer(m_ipBuffer);
			port_text->GetBuffer(m_portBuffer);
			if (m_ipBuffer && m_portBuffer)
			{
				if (m_client.Connect(m_ipBuffer->c_str(), std::stoi(*m_portBuffer)))
				{
					rtd::Handler2D::SetVisibilityAll(false);
					m_ipBuffer = nullptr;
					m_portBuffer = nullptr;
				}
				else
				{
					m_ipBuffer = nullptr;
					m_portBuffer = nullptr;
					ip_text->Reset();
					port_text->Reset();
				}
			}
		}
		else
		{
			sceneHelp::SetupLobbyJoinScreen();
		}

		rtd::Button* exit_button = GET_ELEMENT("exitGameButton", rtd::Button);
		if (exit_button)
		{
			if (exit_button->IsClicked())
			{
				//std::cout << "IMPLEMENT CLEAN SHUT DOWN HERE!\n";
				this->Shutdown();
			}
		}

		rtd::Button* start_button = GET_ELEMENT("startGameButton", rtd::Button);
		if (start_button)
		{
			if (start_button->IsClicked())
			{
				rtd::Handler2D::SetVisibilityAll(false);
				ip_text->SetVisibility(true);
				port_text->SetVisibility(true);
			}
		}

		if (m_client.IsConnected())
		{
			rtd::TextField* lobby_text = GET_ELEMENT("lobbyBuffer", rtd::TextField);
			if (lobby_text)
			{
				lobby_text->SetVisibility(true);
				if (lobby_text->GetBuffer(m_lobbyBuffer))
				{
					this->JoinLobby(std::stoi(*m_lobbyBuffer));
					rtd::Handler2D::Get().Cleanup();
					m_internalState = 1;
				}
			}

			rtd::Button* host_lobby_button = GET_ELEMENT("hostLobby", rtd::Button);
			if (host_lobby_button)
			{
				host_lobby_button->SetVisibility(true);
				if (host_lobby_button->IsClicked())
				{
					rtd::Handler2D::Get().Cleanup();
					this->CreateLobby();
					m_internalState = 1;
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
				sceneHelp::SetupLobbyJoinScreen();
				m_internalState = 0;
			}
		}

		break;
	}
	default:
	{
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

		if (m_gameID == UINT32_MAX)
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
		//strcpy(buffer, "127.0.0.1");
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
				GetScene("Game").ForEachComponent<comp::Tag<CAMERA>>([&](Entity entt, comp::Tag<CAMERA>& t)
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
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
		sceneHelp::SetupInLobbyScreen();
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
		//rtd::Handler2D::Get().DereferenceAllOnce();
		rtd::Handler2D::Get().Cleanup();
		m_internalState = 0;
		LOG_WARNING("Request denied: %s", err.c_str());
		break;
	}
	case GameMsg::Lobby_AcceptedLeave:
	{
		LOG_WARNING("Left Lobby %u", m_gameID);
		m_isLeavingLobby = false;
		m_gameID = -1;
		SetScene("MainMenu");
		break;
	}
	case GameMsg::Game_Start:
	{
		m_internalState = 2;
		rtd::Handler2D::Get().SetVisibilityAll(false);
		rtd::Handler2D::Get().Cleanup();
		//rtd::Handler2D::Get().DereferenceAllOnce();
		SetScene("Game");
		break;
	}
	case GameMsg::Lobby_Update:
	{
		uint32_t nrOfPlayers = 0;
		uint32_t playerID = -1;
		uint8_t state = 0;
		msg >> state >> playerID >> nrOfPlayers;

		rtd::Text* player2Text = GET_ELEMENT("player2text", rtd::Text);
		rtd::Canvas* player2Canvas = GET_ELEMENT("player2canvas", rtd::Canvas);
		rtd::Picture* player2Symbol = GET_ELEMENT("player2_symbol", rtd::Picture);

		rtd::Text* player1Text = GET_ELEMENT("player1text", rtd::Text);
		rtd::Canvas* player1Canvas = GET_ELEMENT("player1canvas", rtd::Canvas);
		rtd::Picture* player1Symbol = GET_ELEMENT("player1_symbol", rtd::Picture);

		if (playerID == 1)
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
		else if (playerID == 2)
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
				uint32_t id;
				msg >> id;
				e.AddComponent<comp::Network>()->id = id;
				break;
			}
			case ecs::Component::TRANSFORM:
			{
				comp::Transform t;
				msg >> t;
				*e.AddComponent<comp::Transform>() = t;
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
				msg >> box.Orientation >> box.Extents >> box.Center;
				*e.AddComponent<comp::BoundingOrientedBox>() = box;
				break;
			}
			case ecs::Component::BOUNDING_SPHERE:
			{
				comp::BoundingSphere s;
				msg >> s.Radius >> s.Center;
				*e.AddComponent<comp::BoundingSphere>() = s;
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

